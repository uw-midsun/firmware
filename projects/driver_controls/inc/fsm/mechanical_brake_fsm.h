#pragma once

// Keeps track of whether the mechanical brake is being pressed or not

<<<<<<< HEAD:projects/driver_controls/inc/mechanical_brake_fsm.h
// Coast, drive, and any events involving cruise control will not be
// acknowledged while
// the machanical brake is pressed
=======
// Coast, drive, and any events involving cruise control will not be acknowledged while
// the mechanical brake is pressed
>>>>>>> b086f57650b8370417cf9e31ce8ed4ac84a0db83:projects/driver_controls/inc/fsm/mechanical_brake_fsm.h

// Direction selector events will not be acknowledged while the mechanical brake
// is not pressed

#include "event_arbiter.h"
#include "fsm.h"

typedef enum {
  MECHANICAL_BRAKE_FSM_STATE_ENGAGED,
  MECHANICAL_BRAKE_FSM_STATE_DISENGAGED
} MechanicalBrakeFSMState;

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage);
