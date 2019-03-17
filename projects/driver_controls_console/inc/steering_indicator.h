#pragma once
// Handles Control Stalk events
// Requires CAN, event queue to be initialized

// Raises INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_ANALOG_CC_RESUME,
// INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
// INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED,
// INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, or
// INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT

#include "can.h"
#include "drive_output.h"
#include "exported_enums.h"
#include "status.h"

// Register CAN rx handler for the steering board
StatusCode steering_indicator_init();
