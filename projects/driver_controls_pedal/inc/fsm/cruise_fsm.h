#pragma once
// Cruise control FSM
//
// This is responsible for handling the changes to the Drive Output messages
// when Cruise Control mode is engaged/disengaged. The target speeds are
// updated as such.
//
// Note: If Cruise Control is active, it overrides the Throttle states.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode cruise_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
