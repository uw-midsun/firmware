#pragma once

#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"

// This module handles state transitions for signal lights. There are 6 states in total.

// Command definitions to turn ON and OFF signal lights.
typedef enum {
  LIGHTS_SIGNAL_CMD_OFF = 0,
  LIGHTS_SIGNAL_CMD_ON,
  NUM_LIGHTS_SIGNAL_CMDS
} LightsSignalFSMCmd;

// Instance of lights signal.
typedef struct LightsSignalFSM {
  LightsBlinker blinker;
  FSM fsm;
} LightsSignalFSM;

// Initializes state machines, and blinker.
StatusCode lights_signal_fsm_init(LightsSignalFSM *lights_signal_fsm,
                                  LightsBlinkerDuration blinker_duration);

// Processes signal events
StatusCode lights_signal_fsm_process_event(LightsSignalFSM *lights_signal_fsm, const Event *event);
