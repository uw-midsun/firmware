#pragma once
// Module for updating the power distribution state.
//
// Requires CAN and event_queue to be initialized.
//
// Chaos will ACK with an affirmative response if the state is valid.
//
// Chaos is guaranteed to transition as the requested state unless an emergency or idle command is
// sent prior to the previous command completing. In particular to get between Drive and Charge
// states the car must pass through the Idle state. Additionally Drive and Charge will fail if the
// car is in the Emergency fault state which is automatically entered in the event of a fault.
//
// The driver should be able to determine whether the transition was successful
// based on the information surfaced via Driver Display.

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

// Update the state of Chaos via CAN.
StatusCode power_distribution_controller_send_update(EEPowerState power_state);
