#pragma once
// Cruise control target speed module
//
// Stores updates to target cruise speed and reads motor speed from the motor controllers.
// Also raises INPUT_EVENT_SPEED_UPDATE events where the data field is the average speed in cm/s.
//
// On events:
//   * INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED:
//      sets the target speed to the current speed
//   * INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS:
//      offsets the target speed by +CRUISE_OFFSET_CMS
//   * INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS:
//      offsets the target speed by -CRUISE_OFFSET_CMS
//
// Requires CAN to be initialized
#include <stdbool.h>
#include <stdint.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

// How to much to increment/decrement in cm/s (m/s * 100)
// Arbitrary default of ~1 km/h
#define CRUISE_OFFSET_CMS 28
// Arbitrary maximum of ~113 km/h for safety
#define CRUISE_MAX_TARGET_CMS 3150

typedef struct CruiseStorage {
  volatile int16_t target_speed_cms;   // m/s * 100
  volatile int16_t current_speed_cms;  // From motor controllers
  int16_t offset_cms;
  SoftTimerId repeat_timer;  // Repeats offset while increment/decrement is held
  size_t repeat_counter;
} CruiseStorage;

// Registers a CAN handler for motor controller speed
StatusCode cruise_init(CruiseStorage *cruise);

// Sets the target speed in cm/s - must be >= 0 (0 is considered disabled)
StatusCode cruise_set_target_cms(CruiseStorage *cruise, int16_t target);

// Returns the current target speed in cm/s.
int16_t cruise_get_target_cms(CruiseStorage *cruise);

// Returns whether the cruise module handled the event
// Handles cruise increment/decrement, set based on current speed
// Note that this function should be called before the FSMs
bool cruise_handle_event(CruiseStorage *cruise, const Event *e);

// Optional storage
CruiseStorage *cruise_global(void);
