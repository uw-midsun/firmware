#pragma once

#include "event_queue.h"

// High priority messages in the event queue
typedef enum {
  PEDAL_EVENT_DUMMY = 0,
  NUM_PEDAL_EVENTS_CRITICAL,
} PedalEventsCritical;

// CAN related messages in the event queue
typedef enum {
  PEDAL_EVENT_CAN_FAULT = NUM_PEDAL_EVENTS_CRITICAL + 1,
  PEDAL_EVENT_CAN_RX,
  PEDAL_EVENT_CAN_TX,
  NUM_PEDAL_EVENTS_CAN
} PedalEventsCAN;

// State transition messages in the event queue
typedef enum {
  PEDAL_EVENT_INPUT_MECH_BRAKE_PRESSED = NUM_PEDAL_EVENTS_CAN + 1,
  PEDAL_EVENT_INPUT_MECH_BRAKE_RELEASED,

  PEDAL_EVENT_INPUT_BPS_FAULT,

  PEDAL_EVENT_INPUT_CENTER_CONSOLE_POWER,
  PEDAL_INPUT_EVENT_
} PedalEventsInput;

// Steering Board
typedef enum {
  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_PRESSED,
  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_RELEASED,

  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_ON,
  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_OFF,

  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,
  PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED,

  NUM_PEDAL_EVENTS_CONTROL_STALK_DIGITAL,
} PedalEventsControlStalkDigital;

typedef enum {
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,

  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,

  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_NEUTRAL,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_CANCEL,
  PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_RESUME,
} PedalEventsInputAnalog;

// Center Console
typedef enum {
  PEDAL_EVENT_INPUT_CENTER_CONSOLE_POWER_PRESSED,

  PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_DRIVE,
  PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_REVERSE,

  PEDAL_EVENT_INPUT_CENTER_CONSOLE_DRL,
  PEDAL_EVENT_INPUT_CENTER_CONSOLE_LOWBEAMS,

  PEDAL_EVENT_INPUT_CENTER_CONSOLE_HAZARDS_PRESSED,
  // PEDAL_EVENT_INPUT_CENTER_CONSOLE_HAZARDS_RELEASED,
} PedalEventsCenterConsole;

typedef enum { PEDAL_EVENT_INPUT_PEDAL_COAST } PedalEventsPedal;