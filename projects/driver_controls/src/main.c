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

#include "cruise.h"
#include "drive_output.h"

#include "can.h"
#include "dc_cfg.h"

typedef StatusCode (*DriverControlsFsmInitFn)(FSM *fsm, EventArbiterStorage *storage);

typedef enum {
  DRIVER_CONTROLS_FSM_POWER = 0,
  DRIVER_CONTROLS_FSM_CRUISE,
  DRIVER_CONTROLS_FSM_PEDAL,
  DRIVER_CONTROLS_FSM_DIRECTION,
  DRIVER_CONTROLS_FSM_MECH_BRAKE,
  NUM_DRIVER_CONTROLS_FSMS
} DriverControlsFsm;

static GpioExpanderStorage s_console_expander;
static CenterConsoleStorage s_console;
static CalibStorage s_calib;
static Ads1015Storage s_pedal_ads1015;
static EventArbiterStorage s_event_arbiter;
static FSM s_fsms[NUM_DRIVER_CONTROLS_FSMS];

static ControlStalk s_stalk;
static GpioExpanderStorage s_stalk_expander;
static Ads1015Storage s_stalk_ads1015;

static CANStorage s_can;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

static void prv_blink_timeout(SoftTimerID timer_id, void *context) {
  debug_led_toggle_state(DEBUG_LED_YELLOW);

  soft_timer_start_seconds(1, prv_blink_timeout, NULL, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  calib_init(&s_calib);

  const CANSettings can_settings = {
    .device_id = DC_CFG_CAN_DEVICE_ID,
    .bitrate = DC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = DC_CFG_CAN_TX,
    .rx = DC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

  can_add_filter(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWER_STATE);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT);
  can_add_filter(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY);

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = DC_CFG_I2C_BUS_SDA,  //
    .scl = DC_CFG_I2C_BUS_SCL,  //
  };

  i2c_init(DC_CFG_I2C_BUS_PORT, &i2c_settings);

  GPIOAddress console_int_pin = DC_CFG_CONSOLE_IO_INT_PIN;
  gpio_expander_init(&s_console_expander, DC_CFG_I2C_BUS_PORT, DC_CFG_CONSOLE_IO_ADDR,
                     &console_int_pin);
  center_console_init(&s_console, &s_console_expander);

  GPIOAddress stalk_int_pin = DC_CFG_STALK_IO_INT_PIN;
  GPIOAddress stalk_ready_pin = DC_CFG_STALK_ADC_RDY_PIN;
  gpio_expander_init(&s_stalk_expander, DC_CFG_I2C_BUS_PORT, DC_CFG_STALK_IO_ADDR, &stalk_int_pin);
  ads1015_init(&s_stalk_ads1015, DC_CFG_I2C_BUS_PORT, DC_CFG_STALK_ADC_ADDR, &stalk_ready_pin);
  control_stalk_init(&s_stalk, &s_stalk_ads1015, &s_stalk_expander);

  GPIOAddress pedal_ads1015_ready = DC_CFG_PEDAL_ADC_RDY_PIN;
  ads1015_init(&s_pedal_ads1015, DC_CFG_I2C_BUS_PORT, DC_CFG_PEDAL_ADC_ADDR, &pedal_ads1015_ready);
  throttle_init(throttle_global(), &calib_blob(&s_calib)->throttle_calib, &s_pedal_ads1015);

  cruise_init(cruise_global());
  drive_output_init(drive_output_global(), INPUT_EVENT_DRIVE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);

  // BPS heartbeat
  bps_indicator_init();
  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  event_arbiter_init(&s_event_arbiter);
  DriverControlsFsmInitFn init_fns[] = {
    cruise_fsm_init, direction_fsm_init, mechanical_brake_fsm_init, pedal_fsm_init, power_fsm_init,
  };
  for (size_t i = 0; i < NUM_DRIVER_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  debug_led_init(DEBUG_LED_YELLOW);
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
        case INPUT_EVENT_CAN_RX:
        case INPUT_EVENT_CAN_TX:
          break;
        default:
          LOG_DEBUG("e %d %d\n", e.id, e.data);
      }
#endif
      can_process_event(&e);
      power_distribution_controller_retry(&e);
      cruise_handle_event(cruise_global(), &e);
      event_arbiter_process_event(&s_event_arbiter, &e);
    }
  }
}
