#pragma once
#include "fsm.h"

// Common type definitions for the FSMs to be used

typedef enum {
  STATE_OFF = 0,
  STATE_BRAKE,
  STATE_COAST,
  STATE_DRIVING,
  STATE_CRUISE_CONTROL,
  STATE_NEUTRAL,
  STATE_FORWARD,
  STATE_REVERSE,
  STATE_NO_SIGNAL,
  STATE_LEFT_SIGNAL,
  STATE_RIGHT_SIGNAL,
  STATE_HAZARD_ON,
  STATE_HAZARD_OFF
} FSMState;

typedef enum {
  INPUT_EVENT_POWER_ON = 0,               // Pedal         00
  INPUT_EVENT_POWER_OFF,                  // Pedal         01
  INPUT_EVENT_GAS_BRAKE,                  // Pedal         02
  INPUT_EVENT_GAS_COAST,                  // Pedal         03
  INPUT_EVENT_GAS_PRESSED,                // Pedal         04
  INPUT_EVENT_CRUISE_CONTROL_ON,          // Pedal         05
  INPUT_EVENT_CRUISE_CONTROL_OFF,         // Pedal         06
  INPUT_EVENT_CRUISE_CONTROL_INC,         // Pedal         07
  INPUT_EVENT_CRUISE_CONTROL_DEC,         // Pedal         08
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, // Direction     09
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,   // Direction     10
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, // Direction     11
  INPUT_EVENT_TURN_SIGNAL_NONE,           // Turn Signal   12
  INPUT_EVENT_TURN_SIGNAL_LEFT,           // Turn Signal   13
  INPUT_EVENT_TURN_SIGNAL_RIGHT,          // Turn Signal   14
  INPUT_EVENT_HAZARD_LIGHT_ON,            // Hazard Light  15
  INPUT_EVENT_HAZARD_LIGHT_OFF,           // Hazard Light  16
  INPUT_EVENT_HORN_PRESSED,
  INPUT_EVENT_HORN_RELEASED,
  INPUT_EVENT_EMERGENCY_STOP,
  INPUT_EVENT_HEADLIGHTS_ON,
  INPUT_EVENT_HEADLIGHTS_OFF,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON
} InputEvent;

typedef struct DriverFSM {
  FSM fsm;
  FSMState state;
} DriverFSM;

typedef struct FSMGroup {
  DriverFSM pedal;
  DriverFSM direction;
  DriverFSM turn_signal;
  DriverFSM hazard_light;
} FSMGroup;
