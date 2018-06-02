#pragma once
// Keeps track of whether the mechanical brake is being pressed or not
//
// Engaged: Block cruise control
//          Note that motor controller interface should handle mech brake/throttle interlocks
// Not Engaged: Block direction changes
#include "event_arbiter.h"
#include "fsm.h"

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage);
