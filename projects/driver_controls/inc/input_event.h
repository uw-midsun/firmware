#pragma once

// Common typedefs a functions for the various FSMs to use.

#include "event_queue.h"

// ID definitions for the driver input events.
typedef enum {
  INPUT_EVENT_POWER = 0,  // Event IDs for driver input events
  INPUT_EVENT_PEDAL_BRAKE,
  INPUT_EVENT_PEDAL_COAST,
  INPUT_EVENT_PEDAL_PRESSED,
  INPUT_EVENT_CRUISE_CONTROL,
  INPUT_EVENT_CRUISE_CONTROL_INC,
  INPUT_EVENT_CRUISE_CONTROL_DEC,
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,
  INPUT_EVENT_TURN_SIGNAL_NONE,
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_HAZARD_LIGHT,
  INPUT_EVENT_MECHANICAL_BRAKE_PRESSED,
  INPUT_EVENT_MECHANICAL_BRAKE_RELEASED,
  INPUT_EVENT_HORN,
  INPUT_EVENT_PUSH_TO_TALK,
  NUM_INPUT_EVENT
} InputEvent;

// Raise an event to broadcast device information over CAN
StatusCode input_event_raise_can(InputEvent device_id, uint8_t device_state, uint16_t device_data);
