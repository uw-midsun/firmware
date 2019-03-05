#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bps_indicator.h"
#include "calib.h"
#include "center_console.h"
#include "control_stalk.h"
#include "debug_led.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "heartbeat_rx.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "power_distribution_controller.h"
#include "soft_timer.h"
#include "throttle.h"

#include "cruise_fsm.h"
#include "direction_fsm.h"
#include "event_arbiter.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"

#include "brake_signal.h"
#include "hazards_fsm.h"
#include "headlight_fsm.h"
#include "horn_fsm.h"
#include "turn_signal_fsm.h"

#include "cruise.h"
#include "drive_output.h"

#include "can.h"
#include "crc32.h"
#include "dc_calib.h"
#include "dc_cfg.h"
#include "flash.h"

typedef StatusCode (*SteeringControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  STEERING_CONTROLS_FSM_CRUISE = 0,
  STEERING_CONTROLS_FSM_TURN_SIGNALS,
  NUM_STEERING_CONTROLS_FSMS,
} SteeringControlsFsm;

static CanStorage s_can;
static Fsm s_fsms[NUM_STEERING_CONTROLS_FSMS];

static ControlStalk s_stalk;
static Ads1015Storage s_stalk_ads1015;

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  // crc32_init();
  // flash_init();

  const CanSettings can_settings = {
    .device_id = SC_CFG_CAN_DEVICE_ID,
    .bitrate = SC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = SC_CFG_CAN_TX,
    .rx = SC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

#ifndef SC_CFG_DISABLE_CONTROL_STALK
  GpioAddress stalk_int_pin = SC_CFG_STALK_IO_INT_PIN;
  GpioAddress stalk_ready_pin = SC_CFG_STALK_ADC_RDY_PIN;
  gpio_expander_init(&s_stalk_expander, SC_CFG_I2C_BUS_PORT, SC_CFG_STALK_IO_ADDR, &stalk_int_pin);
  ads1015_init(&s_stalk_ads1015, SC_CFG_I2C_BUS_PORT, SC_CFG_STALK_ADC_ADDR, &stalk_ready_pin);
  control_stalk_init(&s_stalk, &s_stalk_ads1015, &s_stalk_expander);
#endif

  cruise_init(cruise_global());
  steering_output_init(steering_output_global(), INPUT_EVENT_STEERING_WATCHDOG_FAULT,
                       INPUT_EVENT_STEERING_UPDATE_REQUESTED);

  event_arbiter_init(&s_event_arbiter);
  SteeringControlsFsmInitFn init_fns[] = {
    cruise_fsm_init,
    turn_signal_fsm_init,
    horn_fsm_init,
  };

  for (size_t i = 0; i < NUM_STEERING_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  LOG_DEBUG("Steering Controls initialized\n");

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
#ifdef DC_CFG_DEBUG_PRINT_EVENTS
      switch (e.id) {
        case INPUT_EVENT_DRIVE_UPDATE_REQUESTED:
        case INPUT_EVENT_CAN_RX:
        case INPUT_EVENT_CAN_TX:
          break;
        default:
          LOG_DEBUG("e %d %d\n", e.id, e.data);
      }
#endif
      can_process_event(&e);
      cruise_handle_event(cruise_global(), &e);
      event_arbiter_process_event(&s_event_arbiter, &e);
    }
  }
}
