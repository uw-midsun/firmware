#pragma once
// Handles Mechanical Brake presses and retreives pedal data
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_MECHANICAL_BRAKE_PRESSED or
// INPUT_EVENT_MECHANICAL_BRAKE_RELEASED for Power FSM
#include "can.h"
#include "exported_enums.h"
#include "status.h"
#include "drive_output.h"

// Register CAN rx handler for Mech Brake press
StatusCode pedal_indicator_init();
