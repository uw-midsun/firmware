#pragma once
// Requires can, and event_queue to be initialized.

// This module configures Chaos to listen to CAN messages for signals regarding state transitions:
// Valid States
// - Charge
// - Drive
// - Emergency
// - Idle

#include "status.h"

// Register the CAN handler that listens for state transition messages.
StatusCode state_handler_init(void);
