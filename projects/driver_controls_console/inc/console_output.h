#pragma once
// Console command output module
// Requires soft timers and CAN to be initialized.
//
// Periodically broadcasts a console command to the motor controllers with the current state
// Requires data to be updated periodically to ensure that data is not stale.
#include <stdint.h>
#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#define CONSOLE_OUTPUT_CRUISE_DISABLED_SPEED 0
// Arbitrary timeout - all data should be updated at least once within this timeout period
#define CONSOLE_OUTPUT_WATCHDOG_MS 1000
// How often to request state updates and broadcast console commands
#define CONSOLE_OUTPUT_BROADCAST_MS 100

typedef enum {
  CONSOLE_OUTPUT_SOURCE_DIRECTION = 0,
  NUM_CONSOLE_OUTPUT_SOURCES
} ConsoleOutputSource;

typedef struct ConsoleOutputStorage {
  int16_t data[NUM_CONSOLE_OUTPUT_SOURCES];
  int16_t pedal_requested_data;
  EventId fault_event;
  EventId update_req_event;
  SoftTimerId watchdog_timer;
  SoftTimerId output_timer;
  uint8_t watchdog;
} ConsoleOutputStorage;

// Set the events to be raised in the case of a fault or to request a data update
// Starts periodic console output as disabled
StatusCode console_output_init(ConsoleOutputStorage *storage, EventId fault_event,
                               EventId update_req_event);

// Control whether periodic console output is enabled (ex. disable when the car is off)
// Note that if a fault occurs, periodic console output will be disabled.
StatusCode console_output_set_enabled(ConsoleOutputStorage *storage, bool enabled);

// Throttle and steering angle expect sign-extended 12-bit values.
// Use EEConsoleOutputDirection for direction.
StatusCode console_output_update(ConsoleOutputStorage *storage, ConsoleOutputSource source,
                                 int16_t data);

// Returns a pointer to the global console output storage.
// Note that this only exists because our FSMs already use their context pointers for event arbiters
ConsoleOutputStorage *console_output_global(void);
