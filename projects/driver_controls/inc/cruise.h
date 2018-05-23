#pragma once
// Cruise control target speed module
// Requires CAN to be initialized
// Stores updates to target cruise speed, reads motor speed from the motor controllers
// Set the source to motor controller to record the current speed when driving normally.
// Set the source to stored value to start cruise with the current speed and offset it to change the
// target speed.
#include <stdbool.h>
#include <stdint.h>
#include "event_queue.h"
#include "status.h"

// How to much to increment/decrement in cm/s (m/s * 100)
// Arbitrary default of ~1mph
#define CRUISE_OFFSET_CMS 45

typedef struct CruiseStorage {
  int16_t target_speed_cms;  // m/s * 100
  int16_t current_speed_cms; // From motor controllers
} CruiseStorage;

// Registers a CAN handler for motor controller speed
StatusCode cruise_init(CruiseStorage *cruise);

// Only takes effect if the source is a stored value - will always be >= 0
StatusCode cruise_offset(CruiseStorage *cruise, int16_t offset);

int16_t cruise_get_target_cms(CruiseStorage *cruise);

// Returns whether the cruise module handled the event
// Handles cruise increment/decrement
// Note that this function should be called before the FSMs
bool cruise_handle_event(CruiseStorage *cruise, const Event *e);

// Optional storage
CruiseStorage *cruise_global(void);
