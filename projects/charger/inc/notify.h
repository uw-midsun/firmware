#pragma once
// Module for notifying Power Distribution about charging over CAN and handling commands.
// Requires a generic CAN module to be working.

#include "generic_can.h"
#include "status.h"

// Initializes the permissions module to use GenericCan.
StatusCode notify_init(GenericCan *can, uint32_t send_period_s, uint32_t watchdog_period_s);

// Post a notification periodically (acts as a heartbeat).
void notify_post(void);

// Stops sending notifications.
void notify_cease(void);
