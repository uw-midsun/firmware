#pragma once

// Module for controlling the relays. Processes SOLAR_SENSE_EVENT_RELAY_STATE events and takes
// action. Requires GPIO to be initialized.

#include "status.h"
#include "event_queue.h"

// Initializes the relay GPIO pin.
StatusCode solar_sense_relay_init(void);

// Opens/Closes relay.
StatusCode solar_sense_relay_process_event(const Event *e);
