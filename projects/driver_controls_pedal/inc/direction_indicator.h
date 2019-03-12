#pragma once
<<<<<<< HEAD
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_MECHANICAL_BRAKE_PRESSED or
// INPUT_EVENT_MECHANICAL_BRAKE_RELEASED for Power FSM
#include "can.h"
#include "exported_enums.h"
#include "status.h"

StatusCode direction__indicator_init();
=======
// Handles Direction CAN messages
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_PEDAL_DIRECTION_STATE_NEUTRAL/
// FORWARD/REVERSE event
#include "status.h"

// Register CAN rx handler for Direction changes
StatusCode direction_indicator_init();
>>>>>>> ELEC-580: WIP on direction indicator
