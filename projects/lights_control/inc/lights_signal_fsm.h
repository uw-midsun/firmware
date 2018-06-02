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

typedef struct LightsSignalFsm {
  LightsBlinker blinker;
  FSM fsm;
} LightsSignalFsm;

// Initializes the FSM with a non-syncing blinker.
#define lights_signal_fsm_init_front(fsm, duration) lights_signal_fsm_init(fsm, duration, 0)
// Initializes the FSM with a syncing blinker.
#define lights_signal_fsm_init_rear(fsm, duration, sync_count) \
  lights_signal_fsm_init(fsm, duration, sync_count)

// Initializes state machines, and blinker (use above macros instead).
StatusCode lights_signal_fsm_init(LightsSignalFsm *lights_signal_fsm,
                                  LightsBlinkerDuration const blinker_duration,
                                  LightsBlinkerSyncCount count);

// Processes signal events.
StatusCode lights_signal_fsm_process_event(LightsSignalFsm *lights_signal_fsm, const Event *event);
