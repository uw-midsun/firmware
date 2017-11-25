#pragma once

// Common typedefs a functions for the various FSMs to use.

#include "event_queue.h"

// ID definitions for the driver input events.
// TODO: Ordered by priority due to event queue being priority queue
typedef enum {
  // Critical messages have the highest priority
  INPUT_EVENT_POWER = 0,
  INPUT_EVENT_BMS_FAULT,
  INPUT_EVENT_HAZARD_LIGHT,

  // Events from digital inputs will happen far enough apart that their relative priorities
  // will not matter
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,
  INPUT_EVENT_TURN_SIGNAL_NONE,
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_CRUISE_CONTROL,
  INPUT_EVENT_CRUISE_CONTROL_INC,
  INPUT_EVENT_CRUISE_CONTROL_DEC,
  INPUT_EVENT_HORN,
  INPUT_EVENT_PUSH_TO_TALK,
  INPUT_EVENT_HEADLIGHT,
  INPUT_EVENT_BRAKING_REGEN_INC,
  INPUT_EVENT_BRAKING_REGEN_DEC,

  // Interrupts from analog inputs happen continuously, meaning that multiple events will
  // be put on the queue. They must have lower priority, or the other inputs can be starved 
  INPUT_EVENT_PEDAL_BRAKE,
  INPUT_EVENT_PEDAL_COAST,
  INPUT_EVENT_PEDAL_PRESSED,
  INPUT_EVENT_MECHANICAL_BRAKE_PRESSED,
  INPUT_EVENT_MECHANICAL_BRAKE_RELEASED,

  NUM_INPUT_EVENTS
} InputEvent;
