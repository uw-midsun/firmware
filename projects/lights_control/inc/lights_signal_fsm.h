#pragma once
// This module handles state transitions for signal lights. There are 6 states in total.
// Since this module uses lights_blinker to create the blinking behaviour for the signal lights,
// it will raise LIGHTS_EVENT_GPIO_ON and LIGHTS_EVENT_GPIO_OFF events.

#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"

// Instance of lights signal.

#define LIGHTS_SIGNAL_FSM_NO_SYNC 0

typedef struct LightsSignalFsm {
  LightsBlinker blinker;
  FSM fsm;
} LightsSignalFsm;

// Initializes state machines, and blinker.
// If LIGHTS_SIGNAL_NO_SYNC is passed as sync count, then it will use a non-syncing blinker. Thus
// not raising any sync events.
StatusCode lights_signal_fsm_init(LightsSignalFsm *lights_signal_fsm,
                                  LightsBlinkerDuration blinker_duration, uint32_t count);

// Processes signal events.
StatusCode lights_signal_fsm_process_event(LightsSignalFsm *lights_signal_fsm, const Event *event);
