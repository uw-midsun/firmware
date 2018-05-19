#pragma once
// FSM for the charger
//
// Requires
// - notify
// - charger_pin
// - charger_controller
// To be initialized.
//
// Charger design:
//
// Charger Disconnected:
// - Do nothing
//
// Charger Connected:
// - Notify of connection periodically
//
// Charger Charging:
// - Notify of connection periodically
// - Charge while command is not expired and periodically publish data
// - Exits immediately in case of fault
//
// Transition Table:
// | From \ To    | Disconnected | Connected | Charging |
// | Disconnected |       \      |     Y     |     N    |
// | Connected    |       Y      |     \     |   G(Y)   |
// | Charging     |       Y      |     Y     |     \    |
//
// \ - Self-edge not allowed.
// Y - Allowed.
// N - Not allowed.
// G(Y) - Guarded but allowed.

#include "fsm.h"

// Initializes |fsm| as a charger FSM.
void charger_fsm_init(FSM *fsm);
