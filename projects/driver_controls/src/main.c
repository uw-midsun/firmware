#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

#include "event_arbiter.h"
#include "input_event.h"

#include "direction_fsm.h"
#include "hazard_light_fsm.h"
#include "horn_fsm.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_distribution_controller.h"
#include "power_fsm.h"
#include "push_to_talk_fsm.h"
#include "soft_timer.h"
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

  adc_init(ADC_MODE_CONTINUOUS);

  event_queue_init();

  // Initialize FSMs

  for (;;) {
    if (status_ok(event_process(&e))) {
      // Process the event with the input FSMs
      power_distribution_controller_retry(&e);
    }
  }
}
