#pragma once
// Monitors the current state of the turn signals (Left, Right, or None)

#include "fsm.h"

typedef enum {
  SIGNAL_FSM_STATE_NO_SIGNAL,
  SIGNAL_FSM_STATE_LEFT_SIGNAL,
  SIGNAL_FSM_STATE_RIGHT_SIGNAL,
  SIGNAL_FSM_STATE_HAZARD
} TurnSignalFSMState;

StatusCode turn_signal_fsm_init(FSM *fsm);
