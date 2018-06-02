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

typedef enum EELightType {
  EE_LIGHT_TYPE_HIGH_BEAMS = 0,
  EE_LIGHT_TYPE_LOW_BEAMS,
  EE_LIGHT_TYPE_DRL,
  EE_LIGHT_TYPE_BRAKES,
  EE_LIGHT_TYPE_SIGNAL_RIGHT,
  EE_LIGHT_TYPE_SIGNAL_LEFT,
  EE_LIGHT_TYPE_SIGNAL_HAZARD,
  EE_LIGHT_TYPE_STROBE,
  NUM_EE_LIGHT_TYPES,
} EELightType;

typedef enum EELightsState {
  EE_LIGHT_STATE_OFF = 0,  //
  EE_LIGHT_STATE_ON,       //
  NUM_EE_LIGHT_STATES,     //
} EELightsState;

typedef enum EEHornState {
  EE_HORN_STATE_OFF = 0,  //
  EE_HORN_STATE_ON,       //
  NUM_EE_HORN_STATES,     //
} EEHornState;

typedef enum EERelayState {
  EE_RELAY_STATE_OPEN = 0,
  EE_RELAY_STATE_CLOSE,
  NUM_EE_RELAY_STATES,
} EERelayState;

// Used with the POWER_STATE message sent from driver controls to power distribution to request a
// state change.
typedef enum {
  EE_POWER_STATE_IDLE = 0,
  EE_POWER_STATE_CHARGE,
  EE_POWER_STATE_DRIVE,
  NUM_EE_POWER_STATES,
} EEPowerState;
