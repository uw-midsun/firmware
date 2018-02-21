#pragma once

#include "can.h"
#include "lights_events.h"
#include "lights_gpio.h"

#define CAN_NUM_RX_HANDLERS 1

void lights_can_init(BoardType type);
StatusCode send_sync();

typedef enum {
  ACTION_SIGNAL_RIGHT = 0,
  ACTION_SIGNAL_LEFT,
  ACTION_SIGNAL_HAZARD,
  ACTION_HORN,
  ACTION_HEADLIGHTS,
  ACTION_BRAKES,
  ACTION_STROBE,
  ACTION_SYNC,
  NUM_ACTION_ID
} ActionID;

