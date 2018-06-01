#pragma once
// Charger module for interacting with the charger board over CAN.
//
// Requires CAN to be enabled.
//
// Communication Flow:
// The charger periodically notifies Chaos whether or not it is connected. Whenever Chaos receives a
// "connected" message it is expected to respond with whether or not to proceed with charging.
//
// The charger board expects to receive periodic responses and if it doesn't hear a response a
// watchdog will cease charging as a safety precaution. E.g. in the event of a CAN bus fault.
//
// If the charger is reported connecting and Chaos changes state it will always succeed in sending
// an update about whether or not to charge.

#include <stdbool.h>

#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

// Configure the charger and set up CAN handlers.
StatusCode charger_init(void);

// Sets the charger state.
// - If "closed" charging will only occur when the charger is connected to a power supply.
// - If "opened" charging will not occur regardless of whether the charger is connected to a supply.
StatusCode charger_set_state(EERelaySetState state);

// Process events that changes the charger state.
bool charger_process_event(const Event *e);
