#pragma once

// Monitors the current state of the car's direction selector (Neutral, Forward, or Reverse)

// Coast and drive events are forbidde while the FSM is in the neutral state
// Power off events are forbidden while the FSM is in either the forward or reverse state

#include "fsm.h"

typedef enum {
  DIRECTION_FSM_STATE_NEUTRAL,
  DIRECTION_FSM_STATE_FORWARD,
  DIRECTION_FSM_STATE_REVERSE
} DirectionFSMState;

StatusCode direction_fsm_init(FSM *fsm);
