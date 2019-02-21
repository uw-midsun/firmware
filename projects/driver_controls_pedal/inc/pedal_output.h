#pragma once
// Pedal output module
// Requires soft timers and CAN to be initialized.

#include <stdint.h>
#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

// Arbitrary timeout - all data should be updated at least once within this timeout period
#define PEDAL_OUTPUT_WATCHDOG_MS 1000
// How often to request state updates and broadcast pedal commands
#define PEDAL_OUTPUT_BROADCAST_MS 100

typedef enum {
  PEDAL_OUTPUT_SOURCE_THROTTLE = 0,
  PEDAL_OUTPUT_SOURCE_MECH_BRAKE,
  NUM_PEDAL_OUTPUT_SOURCES
} PedalOutputSource;

typedef struct PedalOutputStorage {
  int16_t data[NUM_PEDAL_OUTPUT_SOURCES];
  int16_t pedal_requested_data;
  EventId fault_event;
  EventId update_req_event;
  SoftTimerId watchdog_timer;
  SoftTimerId output_timer;
  uint8_t watchdog;
} PedalOutputStorage;

// Set the events to be raised in the case of a fault or to request a data update
// Starts periodic pedal output as disabled
StatusCode pedal_output_init(PedalOutputStorage *storage, EventId fault_event,
                             EventId update_req_event);

// Control whether periodic pedal output is enabled (ex. disable when the car is off)
// Note that if a fault occurs, periodic pedal output will be disabled.
StatusCode pedal_output_set_enabled(PedalOutputStorage *storage, bool enabled);

// Throttle and steering angle expect sign-extended 12-bit values.
// Use EEPedalOutputDirection for direction.
StatusCode pedal_output_update(PedalOutputStorage *storage, PedalOutputSource source, int16_t data);

// Returns a pointer to the global pedal output storage.
// Note that this only exists because our FSMs already use their context pointers for event arbiters
PedalOutputStorage *pedal_output_global(void);
