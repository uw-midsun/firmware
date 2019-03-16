#pragma once

// Module for controlling the relays. Processes SOLAR_MASTER_EVENT_RELAY_STATE events and takes
// action. Requires GPIO to be initialized.

#include "event_queue.h"
#include "status.h"

// Initializes the relay GPIO pin.
StatusCode solar_master_relay_init(void);

// Opens/Closes relay.
StatusCode solar_master_relay_process_event(const Event *e);
