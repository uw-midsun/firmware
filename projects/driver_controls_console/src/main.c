#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bps_indicator.h"
#include "cc_input_event.h"
#include "center_console.h"
#include "debug_led.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "heartbeat_rx.h"
#include "i2c.h"
#include "interrupt.h"
#include "led_output.h"
#include "log.h"
#include "pedal_indicator.h"
#include "power_distribution_controller.h"
#include "soft_timer.h"
#include "steering_indicator.h"

#include "cruise_fsm.h"
#include "direction_fsm.h"
#include "event_arbiter.h"
#include "mech_brake_fsm.h"
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
#include "cc_cfg.h"
#include "crc32.h"
#include "flash.h"

typedef StatusCode (*DriverControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  DRIVER_CONTROLS_FSM_POWER = 0,
  DRIVER_CONTROLS_FSM_CRUISE,
  DRIVER_CONTROLS_FSM_PEDAL,
  DRIVER_CONTROLS_FSM_DIRECTION,
  DRIVER_CONTROLS_FSM_MECH_BRAKE,
  DRIVER_CONTROLS_FSM_HEADLIGHT,
  DRIVER_CONTROLS_FSM_TURN_SIGNALS,
  DRIVER_CONTROLS_FSM_HAZARDS,
  DRIVER_CONTROLS_FSM_HORN,
  NUM_DRIVER_CONTROLS_FSMS,
} DriverControlsFsm;

static GpioExpanderStorage s_led_expander;
static CenterConsoleStorage s_console;
static EventArbiterStorage s_event_arbiter;
static Fsm s_fsms[NUM_DRIVER_CONTROLS_FSMS];

static CanStorage s_can;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

static void prv_blink_timeout(SoftTimerId timer_id, void *context) {
  debug_led_toggle_state(DEBUG_LED_GREEN);

  soft_timer_start_seconds(1, prv_blink_timeout, NULL, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  crc32_init();
  flash_init();

  const CanSettings can_settings = {
    .device_id = CC_CFG_CAN_DEVICE_ID,
    .bitrate = CC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_RX,
    .tx_event = INPUT_EVENT_CENTER_CONSOLE_CAN_TX,
    .fault_event = INPUT_EVENT_CENTER_CONSOLE_CAN_FAULT,
    .tx = CC_CFG_CAN_TX,
    .rx = CC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

  can_add_filter(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWER_STATE);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT);
  can_add_filter(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY);
  can_add_filter(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT);
  can_add_filter(SYSTEM_CAN_DEVICE_PEDAL_CONTROLS);

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CC_CFG_I2C_BUS_SDA,  //
    .scl = CC_CFG_I2C_BUS_SCL,  //
  };

  i2c_init(CC_CFG_I2C_BUS_PORT, &i2c_settings);

  gpio_expander_init(&s_led_expander, CC_CFG_I2C_BUS_PORT, CC_CFG_CONSOLE_IO_ADDR, NULL);

  led_output_init(&s_led_expander);
  center_console_init(&s_console);

  // BPS heartbeat
  bps_indicator_init();

  // steering indicator
  steering_indicator_init();

  // pedal indicator
  pedal_indicator_init();

  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  drive_output_init(drive_output_global(), INPUT_EVENT_CENTER_CONSOLE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);

  brake_signal_init();

  event_arbiter_init(&s_event_arbiter);

  DriverControlsFsmInitFn init_fns[] = {
    cruise_fsm_init,      direction_fsm_init, mechanical_brake_fsm_init,
    pedal_fsm_init,       power_fsm_init,     headlight_fsm_init,
    turn_signal_fsm_init, hazards_fsm_init,   horn_fsm_init,
  };
  for (size_t i = 0; i < NUM_DRIVER_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  debug_led_init(DEBUG_LED_GREEN);
  soft_timer_start_seconds(1, prv_blink_timeout, NULL, NULL);

  LOG_DEBUG("Driver Controls initialized\n");

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
#ifdef DC_CFG_DEBUG_PRINT_EVENTS
      switch (e.id) {
        case INPUT_EVENT_PEDAL_ACCEL:
        case INPUT_EVENT_PEDAL_COAST:
        case INPUT_EVENT_PEDAL_BRAKE:
        case INPUT_EVENT_DRIVE_UPDATE_REQUESTED:
        case INPUT_EVENT_CENTER_CONSOLE_CAN_RX:
        case INPUT_EVENT_CENTER_CONSOLE_CAN_TX:
          break;
        default:
          LOG_DEBUG("e %d %d\n", e.id, e.data);
      }
#endif
      can_process_event(&e);
      power_distribution_controller_retry(&e);
      cruise_handle_event(cruise_global(), &e);
      event_arbiter_process_event(&s_event_arbiter, &e);
      brake_signal_process_event(&e);
    }
  }
}
