#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "calib.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "heartbeat_rx.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pc_input_event.h"
#include "soft_timer.h"
#include "throttle.h"

#include "event_arbiter.h"
#include "pedal_output.h"

#include "can.h"
#include "crc32.h"
#include "flash.h"
#include "pc_calib.h"
#include "pc_cfg.h"

static PcCalibBlob s_calib_blob;
static Ads1015Storage s_pedal_ads1015;
static MechBrakeStorage s_mech_brake;
static EventArbiterStorage s_event_arbiter;
static CanStorage s_can;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  crc32_init();
  flash_init();

  const CanSettings can_settings = {
    .device_id = PC_CFG_CAN_DEVICE_ID,
    .bitrate = PC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_PEDAL_CAN_RX,
    .tx_event = INPUT_EVENT_PEDAL_CAN_TX,
    .fault_event = INPUT_EVENT_PEDAL_CAN_FAULT,
    .tx = PC_CFG_CAN_TX,
    .rx = PC_CFG_CAN_RX,
    .loopback = false,
  };

  can_init(&s_can, &can_settings);

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = PC_CFG_I2C_BUS_SDA,  //
    .scl = PC_CFG_I2C_BUS_SCL,  //
  };

  i2c_init(PC_CFG_I2C_BUS_PORT, &i2c_settings);

  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  GpioAddress pedal_ads1015_ready = PC_CFG_PEDAL_ADC_RDY_PIN;
  ads1015_init(&s_pedal_ads1015, PC_CFG_I2C_BUS_PORT, PC_CFG_PEDAL_ADC_ADDR, &pedal_ads1015_ready);
  PcCalibBlob *pc_calib_blob = calib_blob();
  throttle_init(throttle_global(), &pc_calib_blob->throttle_calib, &s_pedal_ads1015);

  const MechBrakeSettings mech_brake_settings = {
    .ads1015 = &s_pedal_ads1015,
    .brake_pressed_threshold_percentage = 55,
    .brake_unpressed_threshold_percentage = 45,
    .bounds_tolerance_percentage = 5,
    .channel = ADS1015_CHANNEL_2,
  };
  mech_brake_init(mech_brake_global(), &mech_brake_settings, &pc_calib_blob->mech_brake_calib);

  pedal_output_init(pedal_output_global(), INPUT_EVENT_PEDAL_WATCHDOG_FAULT,
                    INPUT_EVENT_PEDAL_UPDATE_REQUESTED);
  pedal_output_set_enabled(pedal_output_global(), true);

  LOG_DEBUG("Pedal Controls initialized\n");

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
#ifdef PC_CFG_DEBUG_PRINT_EVENTS
      switch (e.id) {
        case INPUT_EVENT_PEDAL_UPDATE_REQUESTED:
        case INPUT_EVENT_PEDAL_CAN_RX:
        case INPUT_EVENT_PEDAL_CAN_TX:
        default:
          LOG_DEBUG("e %d %d\n", e.id, e.data);
      }
#endif
      can_process_event(&e);
    }
  }
}
