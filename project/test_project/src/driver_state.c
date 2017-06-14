#include "driver_state.h"
#include <stdio.h>

void state_init(FSMGroup* fsm_group) {
	pedal_state_init(&fsm_group->pedal.fsm, fsm_group);
  direction_state_init(&fsm_group->direction.fsm, fsm_group);
  turn_signal_state_init(&fsm_group->turn_signal.fsm, fsm_group);
  hazard_light_state_init(&fsm_group->hazard_light.fsm, fsm_group);

  fsm_group->pedal.state = STATE_OFF;
  fsm_group->direction.state = STATE_NEUTRAL;
  fsm_group->turn_signal.state = STATE_NO_SIGNAL;
  fsm_group->hazard_light.state = STATE_HAZARD_OFF;
}

void state_process_event(FSMGroup* fsm_group, Event* e) {
  if (e->id <= 8) {
    fsm_process_event(&fsm_group->pedal.fsm, e); 
  } else if (e->id <= 11) {
    fsm_process_event(&fsm_group->direction.fsm, e);
  } else if (e->id <= 14) {
    fsm_process_event(&fsm_group->turn_signal.fsm, e);
  } else if (e->id <= 16) {
    fsm_process_event(&fsm_group->hazard_light.fsm, e);
  }
}