#pragma once

// This module controls the strobe's behaviour. It uses lights_blinker to create blinking
// behaviour. It needs to be initialized first.

#include "lights_blinker.h"
#include "lights_events.h"
#include "status.h"

typedef struct LightsStrobe {
  LightsBlinker blinker;
} LightsStrobe;

// Initializes strobe's blinker.
StatusCode lights_strobe_init(LightsStrobe *lights_strobe, LightsBlinkerDuration blinker_duration);

// Processes LIGHTS_EVENT_STROBE_ON and LIGHTS_EVENT_STROBE_OFF events.
StatusCode lights_strobe_process_event(LightsStrobe *lights_strobe, const Event *event);