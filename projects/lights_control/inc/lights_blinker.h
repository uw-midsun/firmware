#pragma once
#include "event_queue.h"
#include "soft_timer.h"

// state definitions for blinker
typedef enum {
  LIGHTS_BLINKER_STATE_OFF = 0,
  LIGHTS_BLINKER_STATE_ON,
  NUM_LIGHTS_BLINKER_STATES
} LightsBlinkerState;

typedef uint32_t LightsBlinkerDuration;

// blinker members
typedef struct LightsBlinker {
  EventID event_id;
  SoftTimerID timer_id;
  LightsBlinkerState state;
  LightsBlinkerDuration duration_us;
} LightsBlinker;

// initializes the blinker
StatusCode lights_blinker_init(LightsBlinker *);

// when turned on, blinker will periodically raise events with the sepcified EventID, each time
// alternating the data field
StatusCode lights_blinker_on_us(LightsBlinker *, LightsBlinkerDuration duration_us, EventID id);

// blinker off will raise the registered event id with data set to 0, and will cancel the
// scheduled timer
StatusCode lights_blinker_off(LightsBlinker *);

// resets the blinker, scheduling a new timer, and setting it state to on
StatusCode lights_blinker_reset(LightsBlinker *);

// returns if the current blinker has active timers
bool lights_blinker_inuse(LightsBlinker *);

#define lights_blinker_on_millis(blinker, duration, event_id) \
  lights_blinker_on_us(blinker, duration * 1000, event_id)
