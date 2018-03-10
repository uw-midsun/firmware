#pragma once

#include "blinker.h"

typedef StatusCode (*StrobeCallback)(Event e);

StatusCode strobe_event_process(Event e);
StatusCode strobe_init(StrobeCallback, BlinkerDuration);

typedef enum {
  STROBE_STATE_OFF = 0,
  STROBE_STATE_ON,
  NUM_STROBE_STATES
} StrobeState;

