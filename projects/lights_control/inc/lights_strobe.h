#pragma once

#include "lights_blinker.h"
#include "lights_events.h"
#include "status.h"

// States of the strobe.
typedef enum LightsStrobeState {
  LIGHTS_STROBE_STATE_OFF = 0,
  LIGHTS_STROBE_STATE_ON,
  NUM_LIGHTS_STROBE_STATES
} LightsStrobeState;

typedef struct LightsStrobe {
  LightsBlinker blinker;
  LightsStrobeState state;
} LightsStrobe;

// Initializes strobe's blinker.
StatusCode lights_strobe_init(LightsStrobe *lights_strobe, LightsBlinkerDuration blinker_duration);

// Processes LIGHTS_EVENT_STROBE event.
StatusCode lights_strobe_process_event(LightsStrobe *lights_strobe, const Event *event);
