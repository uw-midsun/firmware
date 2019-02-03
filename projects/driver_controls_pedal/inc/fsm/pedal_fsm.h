#pragma once
// Monitors the current state of the throttle pedal
//
// Blocks cruise control resume if braking.
//
// Updates throttle drive output state in response to drive output update events.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode pedal_fsm_init(Fsm *fsm, EventArbiterStorage *storage);