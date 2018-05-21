#pragma once
// Shared events for FSMs
#include "event_queue.h"

// ID definitions for the driver input events.
typedef enum {
  INPUT_EVENT_DRIVE_WATCHDOG_FAULT = 0,
  INPUT_EVENT_CAN_RX,
  INPUT_EVENT_CAN_TX,
  INPUT_EVENT_CAN_FAULT,
  INPUT_EVENT_BPS_FAULT,
  // Mechanical brake must take precedence over power so pressing the brake then the power button
  // is handled properly
  INPUT_EVENT_MECHANICAL_BRAKE_PRESSED,
  INPUT_EVENT_DRIVE_UPDATE_REQUESTED,
  INPUT_EVENT_MECHANICAL_BRAKE_RELEASED,
  INPUT_EVENT_POWER,
  INPUT_EVENT_PEDAL_BRAKE,
  INPUT_EVENT_PEDAL_COAST,
  INPUT_EVENT_PEDAL_ACCEL,
  INPUT_EVENT_PEDAL_FAULT,

  // TODO(ELEC-406): Need to rework cruise control system - currently does not function as expected
  INPUT_EVENT_CRUISE_CONTROL,
  INPUT_EVENT_CRUISE_CONTROL_INC,
  INPUT_EVENT_CRUISE_CONTROL_DEC,
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
  INPUT_EVENT_TURN_SIGNAL_NONE,
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,

  // Control Stalk analog events
  INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL,
  INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_MINUS,
  INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_PLUS,
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,
  // ambiguous - depends on digital state
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_DIGITAL,
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL,
  INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME,
  INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
  INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,

  // Control Stalk digital events
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_PRESSED,
  INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_RELEASED,

  INPUT_EVENT_HAZARD_LIGHT,
  INPUT_EVENT_HORN,
  INPUT_EVENT_PUSH_TO_TALK,
  NUM_INPUT_EVENTS
} InputEvent;
