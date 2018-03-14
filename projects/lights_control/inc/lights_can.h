#pragma once

#include "can.h"
#include "lights_events.h"
#include "lights_gpio.h"

#define CAN_NUM_RX_HANDLERS 1

// initializes the lights_can module.
// needs to know whether it's the front light board or
// the rear light board.
void lights_can_init(BoardType type, bool loopback);

// This will get used by the sync module
// Rear board is the board that will be sending the
// sync message to the front board. It raises a sync
// event at the current board, and it transmits a CAN
// sync message.

StatusCode send_sync(void);

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
