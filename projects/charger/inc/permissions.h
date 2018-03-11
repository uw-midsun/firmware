#pragma once
// Module for requesting permission to charge over CAN.
// Requires a generic CAN module to be working.

#include "generic_can.h"
#include "status.h"

// Initializes the permissions module to use GenericCan.
StatusCode permissions_init(GenericCan *can, uint32_t send_period_s, uint32_t watchdog_period_s);

// Request permission periodically.
void permissions_request(void);

// Stops sending requests.
void permissions_cease_request(void);
