#pragma once

// Monitors the current state of the gas pedal

// Brake State: The pedal is depressed, causing the car to brake
// Coast State: The pedal is partially pressed, causing the car to move at a constant speed
// Driving State: The pedal is fully pressed, causing the car to accelerate
// Cruise Control State: Activates cruise control and causes the car to move without the pedal

#include "fsm.h"

void pedal_fsm_init(FSM *pedal_fsm, void *context);
