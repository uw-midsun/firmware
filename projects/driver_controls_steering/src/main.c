#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "can.h"
#include "can_msg_defs.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"

#include "bps_indicator.h"
#include "calib.h"
#include "control_stalk.h"
#include "cruise.h"
#include "debug_led.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "heartbeat_rx.h"
#include "power_state_indicator.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
#include "soft_timer.h"
#include "steering_output.h"

#include "cruise_fsm.h"
#include "horn_fsm.h"
#include "turn_signal_fsm.h"

typedef StatusCode (*SteeringControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  STEERING_CONTROLS_FSM_CRUISE = 0,
  STEERING_CONTROLS_FSM_TURN_SIGNALS,
  STEERING_CONTROLS_FSM_HORN,
  NUM_STEERING_CONTROLS_FSMS,
} SteeringControlsFsm;

static CanStorage s_can;
static Fsm s_fsms[NUM_STEERING_CONTROLS_FSMS];

static ControlStalk s_stalk;
static EventArbiterStorage s_event_arbiter;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

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
    .rx_event = INPUT_EVENT_STEERING_CAN_RX,
    .tx_event = INPUT_EVENT_STEERING_CAN_TX,
    .fault_event = INPUT_EVENT_STEERING_CAN_FAULT,
    .tx = SC_CFG_CAN_TX,
    .rx = SC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

  // Power state
  power_state_indicator_init();

  // BPS heartbeat
  bps_indicator_init();

  // Not sure that this does anything since NULL context is being passed
  // and none of the functions called on callback raise an event
  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  control_stalk_init(&s_stalk);
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
#ifdef SC_CFG_DEBUG_PRINT_EVENTS
      switch (e.id) {
        case INPUT_EVENT_STEERING_UPDATE_REQUESTED:
        case INPUT_EVENT_STEERING_CAN_RX:
        case INPUT_EVENT_STEERING_CAN_TX:
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
