#pragma once
// Handles faults from BPS. Listens for BPS heartbeat.
// Requires CAN, event queue to be initialized
//
// Updates dash indicator and controls the strobe light through CAN.
// Raises BPS fault event if a faulted heartbeat is detected.
#include "can.h"
#include "status.h"

// TODO(ELEC-455): Request pointer to center console to update dash indicator
StatusCode bps_indicator_init(void);

// Mark the fault as cleared - update dash and strobe
StatusCode bps_indicator_clear_fault(void);
