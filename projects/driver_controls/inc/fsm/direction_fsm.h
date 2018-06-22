#pragma once
// Monitors the current state of the car's direction selector (Neutral, Forward, or Reverse)
// Coast events are forbidden while the FSM is in the neutral/reverse state
#include "event_arbiter.h"
#include "fsm.h"

StatusCode direction_fsm_init(FSM *fsm, EventArbiterStorage *storage);
