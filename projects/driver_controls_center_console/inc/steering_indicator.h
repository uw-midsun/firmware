#pragma once
// Handles Control Stalk events
// Requires CAN, event queue to be initialized

#include "can.h"
#include "drive_output.h"
#include "exported_enums.h"
#include "status.h"

// Register CAN rx handler for the steering board
StatusCode steering_indicator_init();
