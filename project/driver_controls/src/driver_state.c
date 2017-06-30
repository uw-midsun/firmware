#include <stdio.h>

#include "driver_state.h"

#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"

typedef bool (*TransitionCheck)(Event *e);

static bool prv_get_permit(FSMGroup *fsm_group, Event *e) {
  bool transitioned = 1;
  
  transitioned &= ((TransitionCheck)fsm_group->power.context)(e);
  transitioned &= ((TransitionCheck)fsm_group->pedal.context)(e);
  transitioned &= ((TransitionCheck)fsm_group->direction.context)(e);
  transitioned &= ((TransitionCheck)fsm_group->turn_signal.context)(e);
  transitioned &= ((TransitionCheck)fsm_group->hazard_light.context)(e);
  transitioned &= ((TransitionCheck)fsm_group->mechanical_brake.context)(e);

  return transitioned;
}

void state_init(FSMGroup *fsm_group) {
  power_state_init(&fsm_group->power, fsm_group);
	pedal_state_init(&fsm_group->pedal, fsm_group);
  direction_state_init(&fsm_group->direction, fsm_group);
  turn_signal_state_init(&fsm_group->turn_signal, fsm_group);
  hazard_light_state_init(&fsm_group->hazard_light, fsm_group);
}

bool state_process_event(FSMGroup *fsm_group, Event *e) {
  if (prv_get_permit(fsm_group, e)) {
    if (fsm_process_event(&fsm_group->power, e)) {
      return true;
    } 
    if (fsm_process_event(&fsm_group->pedal, e)) {
      return true;
    }
    if (fsm_process_event(&fsm_group->direction, e)) {
      return true;
    }
    if (fsm_process_event(&fsm_group->turn_signal, e)) {
      return true;
    }
    if (fsm_process_event(&fsm_group->hazard_light, e)) {
      return true;
    } 
  }

  return 0;
}