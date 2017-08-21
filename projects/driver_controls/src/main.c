#include <stdio.h>
#include <stdint.h>

#include "adc.h"
#include "gpio_it.h"
#include "interrupt.h"

#include "digital_io.h"
#include "analog_io.h"

#include "event_arbiter.h"

#include "power_fsm.h"
#include "pedal_fsm.h"
#include "direction_fsm.h"
#include "turn_signal_fsm.h"
#include "hazard_light_fsm.h"
#include "mechanical_brake_fsm.h"
#include "horn_fsm.h"
#include "push_to_talk_fsm.h"
#include "can_fsm.h"

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
  FSM can;
} FSMGroup;

int main() {
  FSMGroup fsm_group;
  Event e;
  uint16_t reading;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  gpio_it_init();

  adc_init(ADC_MODE_CONTINUOUS);

  digital_io_init();
  analog_io_init();

  event_queue_init();

  // Initialize FSMs
  event_arbiter_init();

  power_fsm_init(&fsm_group.power);
  pedal_fsm_init(&fsm_group.pedal);
  direction_fsm_init(&fsm_group.direction);
  turn_signal_fsm_init(&fsm_group.turn_signal);
  hazard_light_fsm_init(&fsm_group.hazard_light);
  mechanical_brake_fsm_init(&fsm_group.mechanical_brake);
  horn_fsm_init(&fsm_group.horn);
  push_to_talk_fsm_init(&fsm_group.push_to_talk);

  can_fsm_init(&fsm_group.can);

  for (;;) {
    if (status_ok(event_process(&e))) {
      event_arbiter_process_event(&e);
    }
  }
}
