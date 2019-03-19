#pragma once
// Handles Mechanical Brake presses
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_MECHANICAL_BRAKE_PRESSED or
// INPUT_EVENT_MECHANICAL_BRAKE_RELEASED for Power FSM
#include "can.h"
#include "status.h"

// Register CAN rx handler for Mech Brake press
StatusCode pedal_indicator_init();
