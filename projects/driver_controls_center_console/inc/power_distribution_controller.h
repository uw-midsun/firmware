#pragma once
// Module for updating the power distribution state.
//
// Requires CAN and event_queue to be initialized.
//
// Chaos will ACK with an affirmative response if received and it is able to process it.
//
// Chaos is guaranteed to transition as the requested state.
//
// The driver should be able to determine whether the transition was successful
// based on the information surfaced via Driver Display.

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

// Update the state of Chaos via CAN.
StatusCode power_distribution_controller_send_update(EEPowerState power_state);

// Handles retry events in the main loop.
StatusCode power_distribution_controller_retry(const Event *e);
