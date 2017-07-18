#pragma once

// Define common typedefs for the various FSMs to use.

#include "event_queue.h"

// ID definitions for the driver input events.
typedef enum {
  INPUT_EVENT_NONE = 0,                     // Pedal         00
  INPUT_EVENT_POWER,                        // Pedal         01
  INPUT_EVENT_GAS_BRAKE,                    // Pedal         02
  INPUT_EVENT_GAS_COAST,                    // Pedal         03
  INPUT_EVENT_GAS_PRESSED,                  // Pedal         04
  INPUT_EVENT_CRUISE_CONTROL,               // Pedal         05
  INPUT_EVENT_CRUISE_CONTROL_INC,           // Pedal         06
  INPUT_EVENT_CRUISE_CONTROL_DEC,           // Pedal         07
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,   // Direction     08
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,     // Direction     09
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,   // Direction     10
  INPUT_EVENT_TURN_SIGNAL_NONE,             // Turn Signal   11
  INPUT_EVENT_TURN_SIGNAL_LEFT,             // Turn Signal   12
  INPUT_EVENT_TURN_SIGNAL_RIGHT,            // Turn Signal   13
  INPUT_EVENT_HAZARD_LIGHT,                 // Hazard Light  14
  INPUT_EVENT_MECHANICAL_BRAKE,             // Mech. Brake   15
  INPUT_EVENT_HORN,
  INPUT_EVENT_EMERGENCY_STOP,
  INPUT_EVENT_HEADLIGHTS,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON,
  NUM_INPUT_EVENT
} InputEvent;
