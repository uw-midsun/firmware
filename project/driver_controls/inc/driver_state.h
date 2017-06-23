#pragma once
#include "fsm.h"
#include "event_queue.h"
#include "fsm_state.h"
#include "pedal_state.h"

void state_init(FSMGroup* fsm_group);
bool state_process_event(FSMGroup* fsm_group, Event* e);