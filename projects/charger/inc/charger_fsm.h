#pragma once

#include <stdbool.h>

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

void charger_fsm_init(void);

bool charger_fsm_process_event(const Event *e);
