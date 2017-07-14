#pragma once

// Monitors the current state of the turn signals (Left, Right, or None)

#include "fsm.h"

void turn_signal_state_init(FSM *turn_signal_fsm, void *context);
