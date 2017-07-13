#pragma once

// Keeps track of whether the hazard lights are powered

#include "fsm.h"

void hazard_light_state_init(FSM *hazard_light_fsm, void *context);
