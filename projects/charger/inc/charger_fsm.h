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
// | Disconnected |              |     Y     |     N    |
// | Connected    |       Y      |           |   G(Y)   |
// | Charging     |       Y      |           |          |
//
// Y - Allowed
// N - Not Allowed
// G(Y) - Guarded but Allowed

#include <stdbool.h>

#include "charger_can.h"
#include "event_queue.h"

// Initializes the FSM |charger_can_status| is used to guard against faults.
void charger_fsm_init(ChargerCanStatus *charger_can_status);

// Attempts the transition based on the event in |e|.
bool charger_fsm_process_event(const Event *e);
