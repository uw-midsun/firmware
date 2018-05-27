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
  LIGHTS_SIGNAL_TYPE_LEFT = 0,
  LIGHTS_SIGNAL_TYPE_RIGHT,
  LIGHTS_SIGNAL_TYPE_HAZARD,
  NUM_LIGHTS_SIGNAL_TYPES
} LightsSignalType;

typedef enum {
  LIGHTS_GENERIC_TYPE_HIGH_BEAMS = 0,
  LIGHTS_GENERIC_TYPE_LOW_BEAMS,
  LIGHTS_GENERIC_TYPE_DRL,
  LIGHTS_GENERIC_TYPE_BRAKES,
  NUM_LIGHTS_GENERIC_TYPES
} LightsGenericType;

typedef enum { LIGHTS_STATE_OFF = 0, LIGHTS_STATE_ON, NUM_LIGHTS_STATES } LightsState;

typedef enum { HORN_STATE_OFF = 0, HORN_STATE_ON, NUM_HORN_STATES } HornState;

typedef enum BpsHeartbeatState {
  BPS_HEARTBEAT_STATE_OK = 0,
  BPS_HEARTBEAT_STATE_ERROR,
  NUM_BPS_HEARTBEAT_STATES,
} BpsHeartbeatState;
