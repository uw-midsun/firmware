#pragma once
// Handles faults from BMS. Listens for the BMS Heartbeat message, and enables
// the strobe light when a BPS fault is detected via a CAN message to the
// Lights board.
//
// Requires CAN, event queue to be initialized
//
// Note that the indicators must be explicitly set and cleared.
#include "status.h"

StatusCode bps_indicator_init(void);

// Mark fault - update dash and strobe
StatusCode bps_indicator_set_fault(void);

// Mark the fault as cleared - update dash and strobe
StatusCode bps_indicator_clear_fault(void);
