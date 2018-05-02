#pragma once
#include "event_queue.h"
#include "soft_timer.h"
// Blinking requires soft_timers and event_queue to be initialized.

// This module aids with the blinking functionality that some lights require such as signal lights.
// An instance of this module is used to periodically raise events. The data field of the event
// will switch between 1 and 0, corresponding to ON and OFF states. Blinking begins with ON state.

// State definitions for blinker.
typedef enum {
  LIGHTS_BLINKER_STATE_OFF = 0,
  LIGHTS_BLINKER_STATE_ON,
  NUM_LIGHTS_BLINKER_STATES
} LightsBlinkerState;

typedef uint32_t LightsBlinkerDuration;

// Blinker members.
typedef struct LightsBlinker {
  EventID event_id;
  SoftTimerID timer_id;
  LightsBlinkerState state;
  LightsBlinkerDuration duration_ms;  // Duration, in milliseconds
} LightsBlinker;

// Initializes the blinker.
StatusCode lights_blinker_init(LightsBlinker *blinker);

// Starts generating events.
StatusCode lights_blinker_on(LightsBlinker *blinker, LightsBlinkerDuration duration_ms, EventID id);

// Raises an event with OFF state (aka. data 0), cancels the scheduled timer.
StatusCode lights_blinker_off(LightsBlinker *blinker);

// Resets the blinker, scheduling a new timer, and setting it state to ON.
StatusCode lights_blinker_reset(LightsBlinker *blinker);

// Returns if the current blinker has active timers.
bool lights_blinker_inuse(LightsBlinker *blinker);
