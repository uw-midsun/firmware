#pragma once

#include <stdbool.h>

#include "charger_controller.h"
#include "event_queue.h"

// Charger design:
//
// Charger Disconnected:
// - Do nothing
//
// Charger Connected:
// - Request permission periodically
//
// Charger Charging:
// - Request permission periodically
// - Charge while permission is granted and periodically publish data

// Initializes the FSM |charger_status| is used to guard against faults.
void charger_fsm_init(ChargerStatus *charger_status);

// Attempts the transition based on the event in |e|.
bool charger_fsm_process_event(const Event *e);
