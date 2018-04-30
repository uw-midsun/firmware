#pragma once
// Monitors the current state of the gas pedal

// Brake State: The pedal is depressed, causing the car to brake
// Coast State: The pedal is partially pressed, causing the car to move at a
// constant speed
// Driving State: The pedal is fully pressed, causing the car to accelerate
<<<<<<< HEAD:projects/driver_controls/inc/pedal_fsm.h
// Cruise Control State: Activates cruise control and causes the car to move
// without the pedal

=======
// Cruise Control State: Activates cruise control and causes the car to move without the pedal
#include "event_arbiter.h"
>>>>>>> b086f57650b8370417cf9e31ce8ed4ac84a0db83:projects/driver_controls/inc/fsm/pedal_fsm.h
#include "fsm.h"

typedef enum {
  PEDAL_FSM_STATE_BRAKE,
  PEDAL_FSM_STATE_COAST,
  PEDAL_FSM_STATE_DRIVING,
  PEDAL_FSM_STATE_CRUISE_CONTROL
} PedalFSMState;

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage);
