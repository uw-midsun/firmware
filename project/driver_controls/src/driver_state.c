#include <stdio.h>

#include "driver_state.h"

#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"

#define MAX_FSMS 10

static bool prv_get_permit(FSMGroup *fsm_group, Event *e) {
  bool transitioned = 1, status;

  fsm_group->pedal.current_state->output(&fsm_group->pedal, e, &status);
  transitioned &= status;

  fsm_group->direction.current_state->output(&fsm_group->direction, e, &status);
  transitioned &= status;

  fsm_group->turn_signal.current_state->output(&fsm_group->turn_signal, e, &status);
  transitioned &= status;

  fsm_group->hazard_light.current_state->output(&fsm_group->hazard_light, e, &status);
  transitioned &= status;

  return transitioned;
}

void state_init(FSMGroup *fsm_group) {
	pedal_state_init(&fsm_group->pedal, fsm_group);
  direction_state_init(&fsm_group->direction, fsm_group);
  turn_signal_state_init(&fsm_group->turn_signal, fsm_group);
  hazard_light_state_init(&fsm_group->hazard_light, fsm_group);
}

bool state_process_event(FSMGroup *fsm_group, Event *e) {
  if (prv_get_permit(fsm_group, e)) {
    if (e->id <= INPUT_EVENT_CRUISE_CONTROL_DEC) {
      return fsm_process_event(&fsm_group->pedal, e);
    } else if (e->id <= INPUT_EVENT_DIRECTION_SELECTOR_REVERSE) {
      return fsm_process_event(&fsm_group->direction, e);
    } else if (e->id <= INPUT_EVENT_TURN_SIGNAL_RIGHT) {
      return fsm_process_event(&fsm_group->turn_signal, e);
    } else if (e->id <= INPUT_EVENT_HAZARD_LIGHT) {
      return fsm_process_event(&fsm_group->hazard_light, e);
    }
  }

  return 0;
}