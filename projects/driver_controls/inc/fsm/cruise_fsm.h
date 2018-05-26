#pragma once
// Cruise control FSM
//
// Note that the cruise FSM sets drive output cruise states. If set, this value should override the
// pedal state.
#include "fsm.h"
#include "event_arbiter.h"

StatusCode cruise_fsm_init(FSM *fsm, EventArbiterStorage *storage);
