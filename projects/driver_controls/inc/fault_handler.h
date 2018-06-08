#pragma once
// Handles faults from BPS. Listens for BPS heartbeat.
// Requires CAN, event queue to be initialized
//
// Updates dash indicator and controls the strobe light through CAN.
// Raises BPS fault event if a faulted heartbeat is detected.
#include "can.h"
#include "status.h"

typedef struct FaultHandlerStorage {
  // TODO(ELEC-455): center console output
} FaultHandlerStorage;

// TODO(ELEC-455): Request pointer to center console to update dash indicator
StatusCode fault_handler_init(FaultHandlerStorage *storage);

// Mark the fault as cleared - update dash and strobe
StatusCode fault_handler_clear_fault(FaultHandlerStorage *storage);
