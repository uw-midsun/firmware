#pragma once

// Keeps track of whether the hazard lights are powered

#include "fsm.h"

typedef enum {
  HAZARD_LIGHT_FSM_STATE_ON,
  HAZARD_LIGHT_FSM_STATE_OFF
} HazardLightFSMState;

StatusCode hazard_light_fsm_init(FSM *fsm);
