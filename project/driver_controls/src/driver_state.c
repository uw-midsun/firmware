#include "driver_state.h"
#include <stdio.h>

void state_init(FSMGroup *fsm_group) {
	pedal_state_init(&fsm_group->pedal.fsm, fsm_group);
  direction_state_init(&fsm_group->direction.fsm, fsm_group);
  turn_signal_state_init(&fsm_group->turn_signal.fsm, fsm_group);
  hazard_light_state_init(&fsm_group->hazard_light.fsm, fsm_group);

  fsm_group->pedal.state = STATE_OFF;
  fsm_group->direction.state = STATE_NEUTRAL;
  fsm_group->turn_signal.state = STATE_NO_SIGNAL;
  fsm_group->hazard_light.state = STATE_HAZARD_OFF;
}

bool state_process_event(FSMGroup *fsm_group, Event *e) {
  bool transitioned;

  if (e->id <= INPUT_EVENT_CRUISE_CONTROL_DEC) {
    transitioned = fsm_process_event(&fsm_group->pedal.fsm, e);
  } else if (e->id <= INPUT_EVENT_DIRECTION_SELECTOR_REVERSE) {
    transitioned = fsm_process_event(&fsm_group->direction.fsm, e);
  } else if (e->id <= INPUT_EVENT_TURN_SIGNAL_RIGHT) {
    transitioned = fsm_process_event(&fsm_group->turn_signal.fsm, e);
  } else if (e->id <= INPUT_EVENT_HAZARD_LIGHT) {
    transitioned = fsm_process_event(&fsm_group->hazard_light.fsm, e);
  }

  return transitioned;
}