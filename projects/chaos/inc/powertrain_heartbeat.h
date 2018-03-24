#pragma once

#include <stdbool.h>

#include "can_ack.h"
#include "event_queue.h"
#include "status.h"

#define POWERTRAIN_HEARTBEAT_MS 500
#define POWERTRAIN_HEARTBEAT_WATCHDOG_MS 1050

StatusCode powertrain_heartbeat_init(void);

bool powertrain_heartbeat_process_event(const Event *e);
