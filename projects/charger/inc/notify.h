#pragma once
// Module for notifying Power Distribution about charging over CAN and handling commands.
//
// Requires a generic CAN module, event queue and soft timers to be running.
//
// Communication Flow:
// The charger periodically notifies Chaos whether or not it is connected. Whenever Chaos receives a
// "connected" message it is expected to respond with whether or not to proceed with charging.
//
// The charger board expects to receive periodic responses and if it doesn't hear a response a
// watchdog will cease charging as a safety precaution. E.g. in the event of a CAN bus fault.

#include "generic_can.h"
#include "status.h"

// Initializes the permissions module to use GenericCan.
StatusCode notify_init(GenericCan *can, uint32_t send_period_s, uint32_t watchdog_period_s);

// Post a connected notification periodically (acts as a heartbeat).
void notify_post(void);

// Stops sending connected notifications, send a single disconnect notification.
void notify_cease(void);
