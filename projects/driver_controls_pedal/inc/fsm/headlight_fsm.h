#pragma once
// Keeps track of whether the headlights are on
//
// Raises PEDAL_EVENT_INPUT_HEADLIGHT_STATE_* events
#include "event_arbiter.h"
#include "fsm.h"

StatusCode headlight_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
