#pragma once
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_MECHANICAL_BRAKE_PRESSED or
// INPUT_EVENT_MECHANICAL_BRAKE_RELEASED for Power FSM
#include "status.h"

StatusCode direction_indicator_init();
