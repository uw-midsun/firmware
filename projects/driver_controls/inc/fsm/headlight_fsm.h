#pragma once
// Keeps track of whether the headlights are on
#include "event_arbiter.h"
#include "fsm.h"
//
// Raises INPUT_EVENT_HEADLIGHT_STATE_* events
StatusCode headlight_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
