#pragma once
// This module handles state transitions for signal lights. There are 6 states in total.

#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"

// Instance of lights signal.
typedef struct LightsSignalFsm {
  LightsBlinker blinker;
  FSM fsm;
} LightsSignalFsm;

// Initializes state machines, and blinker.
StatusCode lights_signal_fsm_init(LightsSignalFsm *lights_signal_fsm,
                                  LightsBlinkerDuration blinker_duration);

// Processes signal events.
StatusCode lights_signal_fsm_process_event(LightsSignalFsm *lights_signal_fsm, const Event *event);
