#pragma once
// Cruise control target speed module
// Requires CAN to be initialized
// Stores updates to target cruise speed and reads motor speed from the motor controllers.
// On events:
// * INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED: Sets the target speed to the current speed
// * INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS: Offsets the target speed by +CRUISE_OFFSET_CMS
// * INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS: Offsets the target speed by -CRUISE_OFFSET_CMS
#include <stdbool.h>
#include <stdint.h>
#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

// How to much to increment/decrement in cm/s (m/s * 100)
// Arbitrary default of ~1mph (~1.61kph)
#define CRUISE_OFFSET_CMS 45
// Arbitrary maximum of ~70mph (~112.65kph)
#define CRUISE_MAX_TARGET_CMS 3150

typedef struct CruiseStorage {
  volatile int16_t target_speed_cms;   // m/s * 100
  volatile int16_t current_speed_cms;  // From motor controllers
  int16_t offset_cms;
  SoftTimerID repeat_timer;  // Repeats offset while increment/decrement is held
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
