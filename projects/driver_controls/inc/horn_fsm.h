#pragma once

// Keeps track of whether the horn is on or not

#include "fsm.h"

typedef enum { HORN_FSM_STATE_OFF, HORN_FSM_STATE_ON } HornFSMState;

StatusCode horn_fsm_init(FSM *fsm);
