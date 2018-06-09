#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "center_console.h"
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
#include "calib.h"

#include "event_arbiter.h"
#include "cruise_fsm.h"
#include "direction_fsm.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"

static GpioExpanderStorage s_expander;
static CenterConsoleStorage s_console;
static ThrottleStorage s_throttle;
static CalibStorage s_calib;
static Ads1015Storage s_ads1015_pedal;
static EventArbiterStorage s_event_arbiter;

typedef StatusCode (*DriverControlsFsmInitFn)(FSM *fsm, EventArbiterStorage *storage);
typedef enum {
  DRIVER_CONTROLS_FSM_POWER = 0,
  DRIVER_CONTROLS_FSM_CRUISE,
  DRIVER_CONTROLS_FSM_PEDAL,
  DRIVER_CONTROLS_FSM_DIRECTION,
  DRIVER_CONTROLS_FSM_MECH_BRAKE,
  NUM_DRIVER_CONTROLS_FSMS
} DriverControlsFsm;

static FSM s_fsms[NUM_DRIVER_CONTROLS_FSMS];

int main() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  calib_init(&s_calib);

  const I2CSettings settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = { GPIO_PORT_B, 9 },  //
    .scl = { GPIO_PORT_B, 8 },  //
  };

  i2c_init(I2C_PORT_1, &settings);

  GPIOAddress console_int_pin = { GPIO_PORT_A, 9 };
  gpio_expander_init(&s_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_1, &console_int_pin);

  GPIOAddress pedal_ads1015_ready = { GPIO_PORT_A, 10 };
  ads1015_init(&s_ads1015_pedal, I2C_PORT_1, ADS1015_ADDRESS_GND, &pedal_ads1015_ready);
  throttle_init(&s_throttle, &calib_blob(&s_calib)->throttle_calib, &s_ads1015_pedal);

  center_console_init(&s_console, &s_expander);

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
      // Process the event with the input FSMs
      power_distribution_controller_retry(&e);
      event_arbiter_process_event(&s_event_arbiter, &e);
    }
  }
}
