#pragma once

// Monitors the current state of the turn signals (Left, Right, or None)

#include "fsm.h"
#include "event_arbiter.h"

typedef enum {
  TURN_SIGNAL_FSM_STATE_NO_SIGNAL,
  TURN_SIGNAL_FSM_STATE_LEFT_SIGNAL,
  TURN_SIGNAL_FSM_STATE_RIGHT_SIGNAL
} TurnSignalFSMState;

StatusCode turn_signal_fsm_init(FSM *fsm, EventArbiterStorage *storage);
