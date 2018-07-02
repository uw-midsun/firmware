#pragma once
// Module for sending periodic heartbeats to the powertrain components:
// - battery management
// - driver controls
// - motor controller
// The heartbeat is sent only when in the Drive state with the intent of proactively entering the
// emergency state if one of the core components required to power and control the vehicle fail.
//
// Expects soft_timer, can, interrupts to be enabled.
#include <stdbool.h>

#include "can_ack.h"
#include "event_queue.h"
#include "status.h"

#define POWERTRAIN_HEARTBEAT_SEQUENTIAL_PACKETS 5
#define POWERTRAIN_HEARTBEAT_MS 1000
#define POWERTRAIN_HEARTBEAT_WATCHDOG_MS 4100

// Sets up powertrain heartbeat
StatusCode powertrain_heartbeat_init(void);

// Processes |e| and acts accordingly. In the event the car successfully enters the Drive state then
// start the heartbeat. Otherwise if the car changes to any other state (Charge, Idle, Emergency)
// then stop the heartbeat.
bool powertrain_heartbeat_process_event(const Event *e);
