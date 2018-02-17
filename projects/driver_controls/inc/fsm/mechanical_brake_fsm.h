#pragma once

// Keeps track of whether the mechanical brake is being pressed or not

// Coast, drive, and any events involving cruise control will not be acknowledged while
// the mechanical brake is pressed

// Direction selector events will not be acknowledged while the mechanical brake is not pressed

#include "fsm.h"
#include "event_arbiter.h"

typedef enum {
  MECHANICAL_BRAKE_FSM_STATE_ENGAGED,
  MECHANICAL_BRAKE_FSM_STATE_DISENGAGED
} MechanicalBrakeFSMState;

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage);
