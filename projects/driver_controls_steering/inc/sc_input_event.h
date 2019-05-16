#pragma once

#include "event_queue.h"

typedef enum {
  // CAN events
  INPUT_EVENT_STEERING_CAN_RX,
  INPUT_EVENT_STEERING_CAN_TX,
  INPUT_EVENT_STEERING_CAN_FAULT,

  NUM_STEERING_INPUT_EVENTS,
} InputEvent;
