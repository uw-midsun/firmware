#pragma once
// Retreives Throttle and Mech Brake position and raises pedal related events
// Requires CAN, event queue to be initialized

#include "can.h"
#include "drive_output.h"
#include "exported_enums.h"
#include "status.h"

// Register CAN rx handler for Mech Brake press
StatusCode pedal_indicator_init();