#pragma once

#include <stdbool.h>

#include "can_ack.h"
#include "event_queue.h"
#include "status.h"

StatusCode powertrain_heartbeat_init(void);

bool powertrain_heartbeat_process_event(const Event *e);
