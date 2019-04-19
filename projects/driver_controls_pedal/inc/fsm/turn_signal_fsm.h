#pragma once
// Monitors the current state of the turn signals (Left, Right, or None)
// This FSM outputs turn signal state over CAN.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode turn_signal_fsm_init(FSM *fsm, EventArbiterStorage *storage);
