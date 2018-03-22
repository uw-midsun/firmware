#pragma once

typedef enum {
  // front board events
  LIGHTS_EVENT_HORN = 0,
  LIGHTS_EVENT_HIGH_BEAMS,
  LIGHTS_EVENT_LOW_BEAMS,
  LIGHTS_EVENT_DRL,
  // rear board events
  LIGHTS_EVENT_BRAKES,
  LIGHTS_EVENT_STROBE,
  // both boards
  LIGHTS_EVENT_SIGNAL_LEFT,
  LIGHTS_EVENT_SIGNAL_RIGHT,
  LIGHTS_EVENT_SIGNAL_HAZARD,
  LIGHTS_EVENT_SYNC,
  // can internal events
  LIGHTS_EVENT_CAN_RX,
  LIGHTS_EVENT_CAN_TX,
  LIGHTS_EVENT_CAN_FAULT,
  NUM_LIGHTS_EVENTS
} LightsEvent;
