#pragma once
#include "event_queue.h"
#include "soft_timer.h"

// lights_Blinker aids with the blinking functionality that some lights require such as signals.
// Requires soft_timers and event_queue to be initialized.

// An instance of this module is used to periodically raise ON and OFF GPIO events corresponding to
// different peripherals.

// State definitions for blinker.
typedef enum {
  LIGHTS_BLINKER_STATE_OFF = 0,  // When the corresponding peripheral is OFF
  LIGHTS_BLINKER_STATE_ON,       // When the corresponding peripheral is ON
  NUM_LIGHTS_BLINKER_STATES
} LightsBlinkerState;

typedef uint32_t LightsBlinkerDuration;

// Blinker members.
typedef struct LightsBlinker {
  uint16_t event_data;
  SoftTimerID timer_id;
  LightsBlinkerState state;
  LightsBlinkerDuration duration_ms;  // Duration, in milliseconds
} LightsBlinker;

// Initializes the blinker.
StatusCode lights_blinker_init(LightsBlinker *blinker, LightsBlinkerDuration duration_ms);

// Starts generating events. Activates the blinker.
StatusCode lights_blinker_activate(LightsBlinker *blinker, EventID id);

// Raises an OFF event with the existing blinker's peripheral as event data, cancels the scheduled
// timer. Deactivates blinker.
StatusCode lights_blinker_deactivate(LightsBlinker *blinker);

// Only used for syncing purposes. Reschedules the timer, sets the state to ON.
StatusCode lights_blinker_sync_on(LightsBlinker *blinker);
