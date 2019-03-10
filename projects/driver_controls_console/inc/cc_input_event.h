#pragma once
// Shared events for FSMs
#include "event_queue.h"

// ID definitions for the driver input events.
typedef enum {
  INPUT_EVENT_CENTER_CONSOLE_WATCHDOG_FAULT = 0,
  INPUT_EVENT_CENTER_CONSOLE_CAN_RX,
  INPUT_EVENT_CENTER_CONSOLE_CAN_TX,
  INPUT_EVENT_CENTER_CONSOLE_CAN_FAULT,
  INPUT_EVENT_CENTER_CONSOLE_BPS_FAULT,
  INPUT_EVENT_CENTER_CONSOLE_UPDATE_REQUESTED,
  INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_MECH_BRAKE_RELEASED,
  INPUT_EVENT_CENTER_CONSOLE_RETRY_POWER_STATE,

  // Center Console
  INPUT_EVENT_CENTER_CONSOLE_POWER,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED,

  // Power state events
  INPUT_EVENT_CENTER_CONSOLE_POWER_STATE_OFF,
  INPUT_EVENT_CENTER_CONSOLE_POWER_STATE_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_POWER_STATE_CHARGE,
  INPUT_EVENT_CENTER_CONSOLE_POWER_STATE_FAULT,

  // Direction state events
  INPUT_EVENT_DIRECTION_STATE_NEUTRAL,
  INPUT_EVENT_DIRECTION_STATE_FORWARD,
  INPUT_EVENT_DIRECTION_STATE_REVERSE,

  // Center console control stalk events
  INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_ANALOG_CC_RESUME,
  INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED,

  // Center console mech brake events
  INPUT_EVENT_CENTER_CONSOLE_MECHANICAL_BRAKE_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_MECHANICAL_BRAKE_RELEASED,

  NUM_INPUT_EVENTS,
} InputEvent;