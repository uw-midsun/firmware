#pragma once
// Module for receiving periodic heartbeats from the BMS. The Center Console
// board needs to be able to indicate to the driver whether a fault has
// occurred via the LED.
//
// If the watchdog ever times out, then a major failure has occurred. The
// intent is that 1-2 failures are forgiven but any more immediately signal a
// fault and the indicator should be turned on. The heartbeat watchdog should
// be restarted if the car recovers from Emergency into Idle but if the BMS
// fault persists will immediately trigger a return to the Idle state within
// milliseconds.
//
// Expects soft_timer, can, interrupts to be enabled.
#include "status.h"

#define BMS_HEARTBEAT_EXPECTED_PERIOD_MS 5000

// Configures BMS heartbeat handler.
StatusCode bms_heartbeat_init(void);
