#pragma once
// Handles faults from BPS. Listens for BPS heartbeat.
// Requires CAN, event queue to be initialized
//
// Enables the strobe light when a BPS fault is detected.
//
// Note that the indicators must be explicitly set and cleared.
#include "status.h"

StatusCode bps_indicator_init(void);

// Mark fault - update dash and strobe
StatusCode bps_indicator_set_fault(void);

// Mark the fault as cleared - update dash and strobe
StatusCode bps_indicator_clear_fault(void);
