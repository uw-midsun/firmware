#pragma once

#include "can_msg_defs.h"

// This file stores enums which are exported between projects to allow both sides to use the same
// enums when sending and receiving CAN Messages over the primary network. To make things easier all
// enums in this file must follow a slightly modified naming convention.
//
// Example:
// typedef enum {
//   EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<VALUE> = 0,
//   // ...
//   NUM_EE_<MY_CAN_MESSAGE_NAME>_<FIELD_NAME>_<PLURAL>,
// } EE<MyCanMessageName><FieldName>

typedef enum {
  EE_CHARGER_SET_RELAY_STATE_OPEN = 0,
  EE_CHARGER_SET_RELAY_STATE_CLOSE,
  NUM_EE_CHARGER_SET_RELAY_STATES,
} EEChargerSetRelayState;

typedef enum {
  EE_CHARGER_CONN_STATE_DISCONNECTED = 0,
  EE_CHARGER_CONN_STATE_CONNECTED,
  NUM_EE_CHARGER_CONN_STATES,
} EEChargerConnState;

typedef enum {
  EE_DRIVER_CONTROLS_FORWARD,
  EE_DRIVER_CONTROLS_REVERSE,
} EEDriverControlsDirection;

typedef enum {
  EE_DRIVER_CONTROLS_BRAKE_DISENGAGED,
  EE_DRIVER_CONTROLS_BRAKE_ENGAGED,
} EEDriverControlsBrakeState;

#define EE_DRIVER_CONTROLS_PEDAL_DENOMINATOR 2048

#define EE_CONVERT_THROTTLE_READING_TO_PERCENTAGE(raw_throttle) \
  ((float)(raw_throttle) / EE_DRIVER_CONTROLS_PEDAL_DENOMINATOR)
