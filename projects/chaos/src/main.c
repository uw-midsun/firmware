// WARNING: Temporary test main! Not for use in vehicle.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_config.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "gpio_seq.h"
#include "interrupt.h"
#include "log.h"
#include "power_path.h"
#include "soft_timer.h"
#include "status.h"

#define CHAOS_NUM_RX_HANDLERS 1

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CHAOS_NUM_RX_HANDLERS];

static StatusCode prv_handle_uvov(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)ack_reply;
  (void)context;
  uint8_t dcdc_ov = 0;
  uint8_t dcdc_uv = 0;
  uint8_t aux_ov = 0;
  uint8_t aux_uv = 0;
  CAN_UNPACK_OVUV_DCDC_AUX(msg, &dcdc_ov, &dcdc_uv, &aux_ov, &aux_uv);
  LOG_DEBUG("Caught UVOV Message | AUX - UV: %d OV: %d | DCDC - UV: %d OV: %d\n", aux_uv, aux_ov,
            dcdc_uv, dcdc_ov);
  return STATUS_CODE_OK;
}

// Temporary debug main for testing.
int main(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  can_init(&can_settings, &s_can_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX, prv_handle_uvov, NULL);

  ChaosConfig *cfg = chaos_config_load();
  const GPIOAddress addrs[] = {
    cfg->telemetry_power,   cfg->array_sense_power,     cfg->rear_camera_power,
    cfg->themis_power,      cfg->driver_display_power,  cfg->front_lights_power,
    cfg->battery_box_power, cfg->motor_interface_power, cfg->rear_lights_power,
    cfg->pjb_fan,           cfg->spare_protected_power, cfg->spare_unprotected_power,
  };

  const GPIOSettings settings = {
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .direction = GPIO_DIR_OUT,
    .resistor = GPIO_RES_NONE,
  };

  StatusCode code =
      gpio_seq_init_pins(addrs, SIZEOF_ARRAY(addrs), &settings, CHAOS_CONFIG_GPIO_OPEN_DELAY_US);
  if (!status_ok(code)) {
    LOG_CRITICAL("Failed to activate GPIO pins. StatusCode: %d\n", code);
    return EXIT_FAILURE;
  }

  code = power_path_init(&cfg->power_path);
  if (!status_ok(code)) {
    LOG_CRITICAL("Failed to activate Power Path. StatusCode: %d\n", code);
    return EXIT_FAILURE;
  }

  code =
      power_path_source_monitor_enable(&cfg->power_path.aux_bat, CHAOS_CONFIG_POWER_PATH_PERIOD_US);
  if (!status_ok(code)) {
    LOG_CRITICAL("Failed to activate Power Path Source AUX BAT. StatusCode: %d\n", code);
    return EXIT_FAILURE;
  }

  code = power_path_source_monitor_enable(&cfg->power_path.dcdc, CHAOS_CONFIG_POWER_PATH_PERIOD_US);
  if (!status_ok(code)) {
    LOG_CRITICAL("Failed to activate Power Path Source DCDC. StatusCode: %d\n", code);
    return EXIT_FAILURE;
  }

  Event e;
  while (true) {
    do {
      code = event_process(&e);
    } while (code != STATUS_CODE_OK);
    fsm_process_event(CAN_FSM, &e);
  }

  return EXIT_SUCCESS;
}
