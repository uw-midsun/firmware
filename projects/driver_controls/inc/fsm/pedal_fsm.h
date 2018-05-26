#pragma once
// Monitors the current state of the throttle pedal
//
// Brake State: Proportional regen braking should be applied
// Coast State: No motor torque should be applied
// Accel State: Proportional positive motor torque should be applied
//
// Updates throttle drive output state in response to drive output update events.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage);
