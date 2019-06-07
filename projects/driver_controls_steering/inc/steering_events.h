#pragma once

#include "event_queue.h"

// High priority messages in the event queue
typedef enum {
  STEERING_EVENT_DUMMY = 0,
  NUM_STEERING_EVENTS_CRITICAL,
} SteeringEventsCritical;

// CAN related messages in the event queue
typedef enum {
  STEERING_EVENT_CAN_FAULT = NUM_STEERING_EVENTS_CRITICAL + 1,
  STEERING_EVENT_CAN_RX,
  STEERING_EVENT_CAN_TX,
  NUM_STEERING_EVENTS_CAN,
} SteeringEventsCAN;

// State transition messages in the event queue
