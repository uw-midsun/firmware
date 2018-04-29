#pragma once
// Charger module for interacting with the charger board over CAN.

#include "status.h"

typedef enum {
  CHARGER_STATE_DISABLED = 0,
  CHARGER_STATE_ENABLED,
  NUM_CHARGER_STATES,
} ChargerState;

// Configure the charger and set up CAN handlers.
StatusCode charger_init(void);

// Sets the charger state.
// - If enabled it will only be active when the charger is connected to the charger board.
// - If disabled it will always be off regardless of whether a charger is connected.
StatusCode charger_set_state(ChargerState state);
