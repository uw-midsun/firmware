#pragma once
// Drive command output module
// Requires soft timers and CAN to be initialized.
//
// Periodically broadcasts a drive command to the motor controllers with the current state
// Requires data to be updated periodically to ensure that data is not stale.
#include <stdint.h>
#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#define STEERING_OUTPUT_CRUISE_DISABLED_SPEED 0
// Arbitrary timeout - all data should be updated at least once within this timeout period
#define STEERING_OUTPUT_WATCHDOG_MS 1000
// How often to request state updates and broadcast drive commands
#define STEERING_OUTPUT_BROADCAST_MS 100

typedef enum { STEERING_OUTPUT_SOURCE_CRUISE, NUM_STEERING_OUTPUT_SOURCES } SteeringOutputSource;

typedef struct SteeringOutputStorage {
  int16_t data[NUM_STEERING_OUTPUT_SOURCES];
  int16_t pedal_requested_data;
  EventId fault_event;
  EventId update_req_event;
  SoftTimerId watchdog_timer;
  SoftTimerId output_timer;
  uint8_t watchdog;
} SteeringOutputStorage;

// Set the events to be raised in the case of a fault or to request a data update
// Starts periodic steering output as disabled
StatusCode steering_output_init(SteeringOutputStorage *storage, EventId fault_event,
                                EventId update_req_event);

// Control whether periodic steering output is enabled (ex. disable when the car is off)
// Note that if a fault occurs, periodic steering output will be disabled.
StatusCode steering_output_set_enabled(SteeringOutputStorage *storage, bool enabled);

StatusCode steering_output_update(SteeringOutputStorage *storage, SteeringOutputSource source,
                                  int16_t data);

// Returns a pointer to the global steering output storage.
// Note that this only exists because our FSMs already use their context pointers for event arbiters
SteeringOutputStorage *steering_output_global(void);