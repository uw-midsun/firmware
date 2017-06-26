#pragma once
#include "fsm.h"

// Common type definitions for the FSMs to be used

typedef enum {
  INPUT_EVENT_POWER = 0,                  // Pedal         00
  INPUT_EVENT_GAS_BRAKE,                  // Pedal         01
  INPUT_EVENT_GAS_COAST,                  // Pedal         02
  INPUT_EVENT_GAS_PRESSED,                // Pedal         03
  INPUT_EVENT_CRUISE_CONTROL,             // Pedal         04
  INPUT_EVENT_CRUISE_CONTROL_INC,         // Pedal         06
  INPUT_EVENT_CRUISE_CONTROL_DEC,         // Pedal         07
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, // Direction     08
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,   // Direction     09
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, // Direction     10
  INPUT_EVENT_TURN_SIGNAL_NONE,           // Turn Signal   11
  INPUT_EVENT_TURN_SIGNAL_LEFT,           // Turn Signal   12
  INPUT_EVENT_TURN_SIGNAL_RIGHT,          // Turn Signal   13
  INPUT_EVENT_HAZARD_LIGHT,               // Hazard Light  14
  INPUT_EVENT_HORN_PRESSED,
  INPUT_EVENT_HORN_RELEASED,
  INPUT_EVENT_EMERGENCY_STOP,
  INPUT_EVENT_HEADLIGHTS_ON,
  INPUT_EVENT_HEADLIGHTS_OFF,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON
} InputEvent;

typedef struct FSMGroup {
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
} FSMGroup;
