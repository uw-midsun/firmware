#pragma once
// Requires CAN, event queue to be initialized

// Raises a INPUT_EVENT_MECHANICAL_BRAKE_PRESSED or
// INPUT_EVENT_MECHANICAL_BRAKE_RELEASED for Power FSM
#include "can.h"
#include "status.h"
#include "exported_enums.h"

StatusCode direction__indicator_init();