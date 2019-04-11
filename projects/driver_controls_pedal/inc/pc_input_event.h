#pragma once

#include "event_queue.h"

// ID definitions for the pedal CAN events
typedef enum {
  INPUT_EVENT_PEDAL_WATCHDOG_FAULT = 0,
  INPUT_EVENT_PEDAL_CAN_RX,
  INPUT_EVENT_PEDAL_CAN_TX,
  INPUT_EVENT_PEDAL_CAN_FAULT,
  INPUT_EVENT_PEDAL_UPDATE_REQUESTED,

  NUM_CENTER_PEDAL_INPUT_EVENTS,
} InputEvent;
