#pragma once

#include <stdbool.h>

#include "event_queue.h"

// TODO(ELEC-105): Export this to a shared library for use by boards with Relays.
typedef enum {
  RELAY_ID_SOLAR_MASTER_FRONT = 0,
  RELAY_ID_SOLAR_MASTER_REAR,
  RELAY_ID_BATTERY,
  RELAY_ID_MAIN_POWER,
  NUM_RELAY_IDS,
} RelayId;

typedef enum {
  RELAY_STATE_OPEN = 0,
  RELAY_STATE_CLOSE = 1,
} RelayState;

// Initializes the relay FSMs.
void relay_init(bool loopback);

// Updates the relays based on an event.
bool relay_process_event(const Event *e);
