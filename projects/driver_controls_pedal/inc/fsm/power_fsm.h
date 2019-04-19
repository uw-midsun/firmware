#pragma once
// Keeps track of whether the car is currently powered or not
//
// All events aside from the power ON and mechanical brake events are
// forbidden while in the off state
//
// Outputs power state CAN messages on transitions.
#include "event_arbiter.h"
#include "fsm.h"

StatusCode power_fsm_init(FSM *fsm, EventArbiterStorage *storage);
