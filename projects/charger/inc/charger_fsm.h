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
// | From \ To                     | Disconnected | Connected | Charging | Conn, Charg. perm. |
// | Disconnected                  |       \      |     Y     |     N    |         Y          |
// | Connected                     |       Y      |     \     |     N    |         Y          |
// | Connected, charing permission |       Y      |   N_C(Y)  |    G(Y)  |         \          |
// | Charging                      |       Y      |     Y     |     \    |         Y          |
//
// *Error state also exists, should only transition to disconnected
// \ - Self-edge not allowed.
// Y - Allowed.
// N - Not allowed.
// G(Y) - Guarded but allowed.

#include "fsm.h"

// Initializes |fsm| as a charger FSM.
void charger_fsm_init(Fsm *fsm);
