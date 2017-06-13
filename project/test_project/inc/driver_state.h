#pragma once
#include "fsm.h"
#include "event_queue.h"
#include "fsm_state.h"
#include "pedal_state.h"

typedef struct FSMGroup {
  FSM pedal_fsm;
  FSM direction_fsm;
  FSM turn_signal_fsm;
  FSM hazard_light_fsm;
} FSMGroup;

void state_init(FSMGroup* fsm_group, FSMState* default_state);
void state_process_event(FSMGroup* fsm_group, Event* e);