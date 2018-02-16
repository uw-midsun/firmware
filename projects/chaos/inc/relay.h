#pragma once

#include <stdbool.h>

#include "event_queue.h"

// Initializes the relay FSMs.
void relay_init(bool loopback);

// Updates the relays based on an event.
bool relay_process_event(const Event *e);
