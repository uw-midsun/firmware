#include <stdio.h>
#include <stdint.h>

#include "driver_state.h"
#include "soft_timer.h"
#include "driver_device.h"

#include "power_state.h"
#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"
#include "mechanical_brake_state.h"

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

  driver_state_add_fsm(&fsm_group.power, power_state_init);
  driver_state_add_fsm(&fsm_group.pedal, pedal_state_init);
  driver_state_add_fsm(&fsm_group.direction, direction_state_init);
  driver_state_add_fsm(&fsm_group.turn_signal, turn_signal_state_init);
  driver_state_add_fsm(&fsm_group.hazard_light, hazard_light_state_init);
  driver_state_add_fsm(&fsm_group.mechanical_brake, mechanical_brake_state_init);

  // Initialize the various driver control devices
  driver_device_init();

  event_queue_init();
  soft_timer_init();

  for (;;) {
    if (status_ok(event_process(&e))) {
      if (driver_state_process_event(&e)) {
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
