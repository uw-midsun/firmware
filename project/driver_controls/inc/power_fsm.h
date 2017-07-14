#pragma once

// Keeps track of whether the car is currently powered or not

// All events aside from the power ON and mechanical brake events are
// forbidden while in the off state

#include "fsm.h"

void power_fsm_init(FSM *power_fsm, void *context);
