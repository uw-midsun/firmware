#include "plutus_sys.h"
#include <string.h>
#include "crc32.h"
#include "event_queue.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "killswitch.h"
#include "plutus_calib.h"
#include "plutus_event.h"
#include "soft_timer.h"

// Board-specific details
typedef struct PlutusSysSpecifics {
  SystemCanDevice can_device;
  SystemCanMessage relay_msg;
} PlutusSysSpecifics;

// clang-format off
static const PlutusSysSpecifics s_specifics[NUM_PLUTUS_SYS_TYPES] = {
  [PLUTUS_SYS_TYPE_MASTER] = {
    .can_device = SYSTEM_CAN_DEVICE_PLUTUS,
    .relay_msg = SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
  },
  [PLUTUS_SYS_TYPE_SLAVE] = {
    .can_device = SYSTEM_CAN_DEVICE_PLUTUS_SLAVE,
    .relay_msg = SYSTEM_CAN_MESSAGE_BATTERY_RELAY_SLAVE,
  },
};
// clang-format on

static StatusCode prv_init_common(PlutusSysStorage *storage, PlutusSysType type) {
  // Init modules that both boards share

  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CANSettings can_settings = {
    .device_id = s_specifics[type].can_device,
    .bitrate = PLUTUS_CFG_CAN_BITRATE,
    .rx_event = PLUTUS_EVENT_CAN_RX,
    .tx_event = PLUTUS_EVENT_CAN_TX,
    .fault_event = PLUTUS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&storage->can, &can_settings));

  status_ok_or_return(can_add_filter(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT));
  status_ok_or_return(can_add_filter(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT));
  status_ok_or_return(can_add_filter(SYSTEM_CAN_MESSAGE_SET_DISCHARGE_BITSET));
  status_ok_or_return(can_add_filter(s_specifics[type].relay_msg));

  const SequencedRelaySettings relay_settings = {
    .can_msg_id = s_specifics[type].relay_msg,
    .left_relay = PLUTUS_CFG_RELAY_PWR,
    .right_relay = PLUTUS_CFG_RELAY_GND,
    .delay_ms = PLUTUS_CFG_RELAY_DELAY_MS,
  };
  status_ok_or_return(sequenced_relay_init(&storage->relay, &relay_settings));

  // Always respond to powertrain heartbeat
  status_ok_or_return(heartbeat_rx_register_handler(&storage->powertrain_heartbeat_handler,
                                                    SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                                    heartbeat_rx_auto_ack_handler, NULL));

  // TODO(ELEC-439): drive fans using PWM
  GPIOSettings fan_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };
  GPIOAddress fan_1 = PLUTUS_CFG_FAN_1, fan_2 = PLUTUS_CFG_FAN_2;
  gpio_init_pin(&fan_1, &fan_settings);
  gpio_init_pin(&fan_2, &fan_settings);

  return STATUS_CODE_OK;
}

PlutusSysType plutus_sys_get_type(void) {
  const GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN };
  GPIOAddress board_selector = PLUTUS_CFG_BOARD_TYPE_SEL;
  gpio_init_pin(&board_selector, &gpio_settings);

  GPIOState state = NUM_GPIO_STATES;
  gpio_get_state(&board_selector, &state);

  return (state == GPIO_STATE_HIGH) ? PLUTUS_SYS_TYPE_MASTER : PLUTUS_SYS_TYPE_SLAVE;
}

StatusCode plutus_sys_init(PlutusSysStorage *storage, PlutusSysType type) {
  if (type >= NUM_PLUTUS_SYS_TYPES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(*storage));
  storage->type = type;

  status_ok_or_return(prv_init_common(storage, type));

  GPIOAddress killswitch = PLUTUS_CFG_KILLSWITCH;
  if (type == PLUTUS_SYS_TYPE_MASTER) {
    // Master also handles:
    // LTC AFE/ADC
    // BPS Heartbeat TX
    // Killswitch fault
    const LtcAfeSettings afe_settings = {
      .mosi = PLUTUS_CFG_AFE_SPI_MOSI,
      .miso = PLUTUS_CFG_AFE_SPI_MISO,
      .sclk = PLUTUS_CFG_AFE_SPI_SCLK,
      .cs = PLUTUS_CFG_AFE_SPI_CS,

      .spi_port = PLUTUS_CFG_AFE_SPI_PORT,
      .spi_baudrate = PLUTUS_CFG_AFE_SPI_BAUDRATE,
      .adc_mode = PLUTUS_CFG_AFE_MODE,

      .cell_bitset = PLUTUS_CFG_CELL_BITSET_ARR,
      .aux_bitset = PLUTUS_CFG_AUX_BITSET_ARR,
    };
    status_ok_or_return(ltc_afe_init(&storage->ltc_afe, &afe_settings));

    crc32_init();
    flash_init();

    PlutusCalibBlob calib_blob = { 0 };
    calib_init(&calib_blob, sizeof(calib_blob), false);

    const LtcAdcSettings adc_settings = {
      .mosi = PLUTUS_CFG_CURRENT_SENSE_MOSI,  //
      .miso = PLUTUS_CFG_CURRENT_SENSE_MISO,  //
      .sclk = PLUTUS_CFG_CURRENT_SENSE_SCLK,  //
      .cs = PLUTUS_CFG_CURRENT_SENSE_CS,      //

      .spi_port = PLUTUS_CFG_CURRENT_SENSE_SPI_PORT,          //
      .spi_baudrate = PLUTUS_CFG_CURRENT_SENSE_SPI_BAUDRATE,  //
      .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,                //
    };
    status_ok_or_return(
        current_sense_init(&storage->current_sense, &calib_blob.current_calib, &adc_settings));

    status_ok_or_return(bps_heartbeat_init(&storage->bps_heartbeat, &storage->relay,
                                           PLUTUS_CFG_HEARTBEAT_PERIOD_MS,
                                           PLUTUS_CFG_HEARTBEAT_EXPECTED_DEVICES));
    status_ok_or_return(
        killswitch_init(&storage->killswitch, &killswitch, &storage->bps_heartbeat));
  } else if (type == PLUTUS_SYS_TYPE_SLAVE) {
    // Slave also handles:
    // BPS Heartbeat RX
    // Bypass killswitch
    //
    // TODO(ELEC-439): fault if heartbeat bad
    status_ok_or_return(heartbeat_rx_register_handler(&storage->bps_heartbeat_handler,
                                                      SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT,
                                                      heartbeat_rx_auto_ack_handler, NULL));
    status_ok_or_return(killswitch_bypass(&killswitch));
  }

  return STATUS_CODE_OK;
}
