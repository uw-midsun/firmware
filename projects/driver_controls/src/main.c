#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"

#include "analog_io.h"
#include "digital_io.h"
#include "driver_io.h"
#include "gpio_expander.h"
#include "event_arbiter.h"
#include "input_event.h"

#include "can_output.h"
#include "direction_fsm.h"
#include "hazard_light_fsm.h"
#include "horn_fsm.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "push_to_talk_fsm.h"
#include "turn_signal_fsm.h"

// Struct of FSMs to be used in the program
typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM horn;
  FSM push_to_talk;
} FSMGroup;

int main() {
  FSMGroup fsm_group;
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(DRIVER_IO_GPIO_EXPANDER_I2C_PORT, &i2c_settings);

  // Initialize GPIO Expander
  GPIOAddress gpio_expander_pin = DRIVER_IO_GPIO_EXPANDER_INTERRUPT;
  gpio_expander_init(gpio_expander_pin, DRIVER_IO_GPIO_EXPANDER_I2C_PORT);

  digital_io_init();

  event_queue_init();

  // Initialize FSMs
  event_arbiter_init(can_output_transmit);

  power_fsm_init(&fsm_group.power);
  pedal_fsm_init(&fsm_group.pedal);
  direction_fsm_init(&fsm_group.direction);
  turn_signal_fsm_init(&fsm_group.turn_signal);
  hazard_light_fsm_init(&fsm_group.hazard_light);
  mechanical_brake_fsm_init(&fsm_group.mechanical_brake);
  horn_fsm_init(&fsm_group.horn);
  push_to_talk_fsm_init(&fsm_group.push_to_talk);

  GPIOState state;

  for (;;) {
    if (status_ok(event_process(&e))) {
      // Process the event with the input FSMs
      event_arbiter_process_event(&e);
    }
  }
}
