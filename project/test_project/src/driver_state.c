#include "driver_state.h"
#include <stdio.h>

void state_init(FSMGroup* fsm_group, FSMState* default_state) {
	pedal_state_init(&fsm_group->pedal_fsm, &default_state[0]);
  direction_state_init(&fsm_group->direction_fsm, &default_state[1]);
  turn_signal_state_init(&fsm_group->turn_signal_fsm, &default_state[2]);
  hazard_light_state_init(&fsm_group->hazard_light_fsm, &default_state[3]);
}

void state_process_event(FSMGroup* fsm_group, Event* e) {
  if (e->id <= 8) {
    fsm_process_event(&fsm_group->pedal_fsm, e); 
  } else if (e->id <= 11) {
    fsm_process_event(&fsm_group->direction_fsm, e);
  } else if (e->id <= 14) {
    fsm_process_event(&fsm_group->turn_signal_fsm, e);
  } else if (e->id <= 16) {
    fsm_process_event(&fsm_group->hazard_light_fsm, e);
  }
}