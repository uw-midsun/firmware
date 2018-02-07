#pragma once

typedef enum {
  // front board events
  EVENT_HORN,
  EVENT_HEADLIGHTS,
  // rear board events
  EVENT_BRAKE,
  EVENT_STROBE,
  // both boards
  EVENT_SIGNAL_LEFT,
  EVENT_SIGNAL_RIGHT,
  EVENT_SIGNAL_HAZARD,
  // can internal events
  EVENT_CAN_RX,
  EVENT_CAN_TX,
  EVENT_CAN_FAULT
} InputEvent;

