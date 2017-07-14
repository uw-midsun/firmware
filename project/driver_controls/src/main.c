#include <stdio.h>
#include <stdint.h>

#include "event_arbiter.h"
#include "driver_io.h"

#include "power_fsm.h"
#include "pedal_fsm.h"
#include "direction_fsm.h"
#include "turn_signal_fsm.h"
#include "hazard_light_fsm.h"
#include "mechanical_brake_fsm.h"

#define INPUT_DEVICES 16
#define OUTPUT_DEVICES 1

// Struct of FSMs to be used in the program
typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
} FSMGroup;

int main() {
  FSMGroup fsm_group;
  Event e;
  uint16_t reading;

  event_arbiter_add_fsm(&fsm_group.power, power_fsm_init);
  event_arbiter_add_fsm(&fsm_group.pedal, pedal_fsm_init);
  event_arbiter_add_fsm(&fsm_group.direction, direction_fsm_init);
  event_arbiter_add_fsm(&fsm_group.turn_signal, turn_signal_fsm_init);
  event_arbiter_add_fsm(&fsm_group.hazard_light, hazard_light_fsm_init);
  event_arbiter_add_fsm(&fsm_group.mechanical_brake, mechanical_brake_fsm_init);

  // Initialize the various driver control devices
  driver_io_init();

  input_interrupt_init();
  event_queue_init();

  for (;;) {
    if (status_ok(event_process(&e))) {
      if (event_arbiter_process_event(&e)) {
        printf("Event = %d   :   %s   :   %s   :   %s   :   %s   :   %s   :   %s\n",
            e.id,
            fsm_group.power.current_state->name,
            fsm_group.pedal.current_state->name,
            fsm_group.direction.current_state->name,
            fsm_group.hazard_light.current_state->name,
            fsm_group.mechanical_brake.current_state->name,
            fsm_group.turn_signal.current_state->name);
      }
    }
  }
}
