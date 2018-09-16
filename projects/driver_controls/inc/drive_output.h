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

#define DRIVE_OUTPUT_CRUISE_DISABLED_SPEED 0
// Arbitrary timeout - all data should be updated at least once within this timeout period
#define DRIVE_OUTPUT_WATCHDOG_MS 1000
// How often to request state updates and broadcast drive commands
#define DRIVE_OUTPUT_BROADCAST_MS 100

typedef enum {
  DRIVE_OUTPUT_SOURCE_THROTTLE = 0,
  DRIVE_OUTPUT_SOURCE_CRUISE,
  DRIVE_OUTPUT_SOURCE_DIRECTION,
  DRIVE_OUTPUT_SOURCE_MECH_BRAKE,
  NUM_DRIVE_OUTPUT_SOURCES
} DriveOutputSource;

typedef struct DriveOutputStorage {
  int16_t data[NUM_DRIVE_OUTPUT_SOURCES];
  int16_t pedal_requested_data;
  EventId fault_event;
  EventId update_req_event;
  SoftTimerId watchdog_timer;
  SoftTimerId output_timer;
  uint8_t watchdog;
} DriveOutputStorage;

// Set the events to be raised in the case of a fault or to request a data update
// Starts periodic drive output as disabled
StatusCode drive_output_init(DriveOutputStorage *storage, EventId fault_event,
                             EventId update_req_event);

// Control whether periodic drive output is enabled (ex. disable when the car is off)
// Note that if a fault occurs, periodic drive output will be disabled.
StatusCode drive_output_set_enabled(DriveOutputStorage *storage, bool enabled);

// Throttle and steering angle expect sign-extended 12-bit values.
// Use EEDriveOutputDirection for direction.
StatusCode drive_output_update(DriveOutputStorage *storage, DriveOutputSource source, int16_t data);

// Returns a pointer to the global drive output storage.
// Note that this only exists because our FSMs already use their context pointers for event arbiters
DriveOutputStorage *drive_output_global(void);
