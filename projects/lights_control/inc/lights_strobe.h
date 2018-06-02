#pragma once

// This module controls the strobe's behaviour. It uses lights_blinker to create blinking
// behaviour. Requires soft_timers, and interrupts to be initialized.

#include "lights_blinker.h"
#include "lights_events.h"
#include "status.h"

typedef struct LightsStrobeStorage {
  LightsBlinker blinker;
} LightsStrobeStorage;

// Initializes strobe's blinker.
StatusCode lights_strobe_init(LightsStrobeStorage *lights_strobe,
                              LightsBlinkerDuration blinker_duration);

// Processes LIGHTS_EVENT_STROBE_ON and LIGHTS_EVENT_STROBE_OFF events.
StatusCode lights_strobe_process_event(LightsStrobeStorage *lights_strobe, const Event *event);
