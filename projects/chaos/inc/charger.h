#pragma once
// Charger module for interacting with the charger board over CAN.

#include "exported_enums.h"
#include "status.h"

// Configure the charger and set up CAN handlers.
StatusCode charger_init(void);

// Sets the charger state.
// - If enabled it will only be active when the charger is connected to the charger board.
// - If disabled it will always be off regardless of whether a charger is connected.
StatusCode charger_set_state(EEChargerSetRelayState state);
