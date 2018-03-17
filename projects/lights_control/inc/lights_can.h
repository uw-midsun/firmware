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
// needs to know whether it's the front light board or
// the rear light board.
void lights_can_init(const CANSettings *);

// This will get used by the sync module
// Rear board is the board that will be sending the
// sync message to the front board. It raises a sync
// event at the current board, and it transmits a CAN
// sync message.
StatusCode send_sync(void);
