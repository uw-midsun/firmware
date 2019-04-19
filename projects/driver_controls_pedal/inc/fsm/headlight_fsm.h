#pragma once
// Keeps track of whether the headlights are on
//
// Raises INPUT_EVENT_HEADLIGHT_STATE_* events
#include "event_arbiter.h"
#include "fsm.h"

StatusCode headlight_fsm_init(FSM *fsm, EventArbiterStorage *storage);
