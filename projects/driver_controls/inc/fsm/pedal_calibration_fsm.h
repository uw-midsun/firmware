#pragma once
// This fsm is used in the calibration process. Every state of the fsm is a stage of calibration.
// The start state is like a menu. User can start calibration or go into validation.
// The full brake state is the stage where data is collected from pedal in full brake position.
// The full throttle state is similar to full brake state.
// The validate state lets the user judge if calibration was successful by putting the pedal in
// different positions and reading the corresponding zones and values on the screen.
#include "event_arbiter.h"
#include "fsm.h"
#include "pedal_calibration.h"

// Initializes the pedal calibration fsm and sets the output functions.
StatusCode pedal_calibration_fsm_init(FSM *fsm, PedalCalibrationStorage *calibration_storage);
