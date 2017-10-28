#pragma once

// Keeps track of whether the car is currently powered or not

// All events aside from the power ON and mechanical brake events are
// forbidden while in the off state

#include "fsm.h"

typedef enum {
  POWER_FSM_STATE_OFF,
  POWER_FSM_STATE_OFF_BRAKE,
  POWER_FSM_STATE_CHARGING,
  POWER_FSM_STATE_ON,
} PowerFSMState;

StatusCode power_fsm_init(FSM *fsm);
