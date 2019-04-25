#pragma once
// Cruise control FSM
//
// Note that the cruise FSM sets drive output cruise states. If set, this value should override the
// pedal state.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode cruise_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
