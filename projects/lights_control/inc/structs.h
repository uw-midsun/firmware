#pragma once

typedef enum {
  LIGHTS_BOARD_FRONT,
  LIGHTS_BOARD_REAR
} BoardType;

typedef enum {
  ACTION_SIGNAL_RIGHT = 0,
  ACTION_SIGNAL_LEFT,
  ACTION_SIGNAL_HAZARD,
  ACTION_HORN,
  ACTION_HEADLIGHTS,
  ACTION_BRAKES,
  ACTION_STROBE,
  NUM_ACTION_ID
} ActionID;
