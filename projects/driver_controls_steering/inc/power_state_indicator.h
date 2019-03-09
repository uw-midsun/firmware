#pragma once
// Handles Power State updates
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_STEERING_POWER_STATE_OFF
// or INPUT_EVENT_STEERING_POWER_STATE_FAULT

#include "status.h"

// Register a CAN rx handler for Power State update
StatusCode power_state_indicator_init();
