#pragma once

#include "can.h"
#include "lights_events.h"

#define CAN_NUM_RX_HANDLERS 1

typedef enum {
  LIGHTS_ACTION_SIGNAL_RIGHT = 0,
  LIGHTS_ACTION_SIGNAL_LEFT,
  LIGHTS_ACTION_SIGNAL_HAZARD,
  LIGHTS_ACTION_HORN,
  LIGHTS_ACTION_HEADLIGHTS,
  LIGHTS_ACTION_BRAKES,
  LIGHTS_ACTION_STROBE,
  LIGHTS_ACTION_SYNC,
  NUM_ACTION_ID
} LightsActionID;

// initializes the lights_can module.
StatusCode lights_can_init(const CANSettings *can_settings);
