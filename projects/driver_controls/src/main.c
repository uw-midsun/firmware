#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "calib.h"
#include "center_console.h"
#include "control_stalk.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
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

static void prv_dump_fsms(void) {
  for (size_t i = 0; i < NUM_DRIVER_CONTROLS_FSMS; i++) {
    printf("> %-30s%s\n", s_fsms[i].name, s_fsms[i].current_state->name);
  }
}

int main() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  calib_init(&s_calib);

  const CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  const I2CSettings settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = { GPIO_PORT_B, 9 },  //
    .scl = { GPIO_PORT_B, 8 },  //
  };

  i2c_init(I2C_PORT_1, &settings);

  GPIOAddress stalk_int_pin = { GPIO_PORT_A, 2 };
  GPIOAddress stalk_ready_pin = { GPIO_PORT_A, 1 };
  gpio_expander_init(&s_stalk_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_0, &stalk_int_pin);
  ads1015_init(&s_stalk_ads1015, I2C_PORT_1, ADS1015_ADDRESS_VDD, &stalk_ready_pin);
  control_stalk_init(&s_stalk, &s_stalk_ads1015, &s_stalk_expander);

  GPIOAddress console_int_pin = { GPIO_PORT_A, 9 };
  gpio_expander_init(&s_console_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_1, &console_int_pin);
  center_console_init(&s_console, &s_console_expander);

  GPIOAddress pedal_ads1015_ready = { GPIO_PORT_A, 10 };
  ads1015_init(&s_pedal_ads1015, I2C_PORT_1, ADS1015_ADDRESS_GND, &pedal_ads1015_ready);
  throttle_init(throttle_global(), &calib_blob(&s_calib)->throttle_calib, &s_pedal_ads1015);

  cruise_init(cruise_global());
  drive_output_init(drive_output_global(), INPUT_EVENT_DRIVE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);

  // TODO(ELEC-455): Add heartbeat handlers

  event_arbiter_init(&s_event_arbiter);
  DriverControlsFsmInitFn init_fns[] = {
    cruise_fsm_init, direction_fsm_init, mechanical_brake_fsm_init, pedal_fsm_init, power_fsm_init,
  };
  for (size_t i = 0; i < NUM_DRIVER_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
      if (e.id != INPUT_EVENT_PEDAL_BRAKE && e.id != INPUT_EVENT_PEDAL_COAST &&
          e.id != INPUT_EVENT_PEDAL_ACCEL) {
        LOG_DEBUG("e %d data %d\n", e.id, e.data);
      }

      fsm_process_event(CAN_FSM, &e);
      power_distribution_controller_retry(&e);
      cruise_handle_event(cruise_global(), &e);
      event_arbiter_process_event(&s_event_arbiter, &e);
    }
  }
}
