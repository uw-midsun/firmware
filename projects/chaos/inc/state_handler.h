#pragma once
// Requires CAN, and event_queue to be initialized.

// This module configures Chaos to listen to CAN messages for signals regarding state transitions:
// Valid States
// - Charge
// - Drive
// - Idle
// NOTE: Emergency while a valid state can only be triggered by Chaos itself based on faults of
// other boards.
//
// The transition is guarded so Chaos will respond with |CAN_ACK_STATUS_INVALID| in the event the
// transition cannot be completed. This occurs under one of two conditions:
// - Chaos is in the middle of a transition and cannot transition at present. Normally this would
//   allow us to enqueue events; however, since we use a pqueue this is unsafe in the event an
//   emergency has occurred.
// - The requested state is not a valid transition from the current state.

#include "event_queue.h"
#include "status.h"

// Register the CAN handler that listens for state transition messages.
StatusCode state_handler_init(void);
