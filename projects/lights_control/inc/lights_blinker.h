#pragma once
#include "event_queue.h"
#include "soft_timer.h"

// This module aids with the blinking functionality that some lights require such as signal lights,
// and strobe. The user of this module will have to provide storage for the blinker instance.
// This module will work by periodically raising events. The module that is using a blinker
// instance will have to provide the event id of their desired event. By convention, the passed
// event name will have the event that the user module handles postfixed with BLINK. For example,
// if the lights_signal module is processing the EVENT_LIGHTS_SIGNAL_RIGHT, it will call this
// blinker with, EVENT_LIGHTS_SIGNAL_RIGHT_BLINK. Blink events get directly handled by the
// lights_gpio module.

// State definitions for blinker
typedef enum {
  LIGHTS_BLINKER_STATE_OFF = 0,
  LIGHTS_BLINKER_STATE_ON,
  NUM_LIGHTS_BLINKER_STATES
} LightsBlinkerState;

typedef uint32_t LightsBlinkerDuration;

// Blinker members
typedef struct LightsBlinker {
  EventID event_id;
  SoftTimerID timer_id;
  LightsBlinkerState state;
  LightsBlinkerDuration duration;  // in milliseconds
} LightsBlinker;

// Initializes the blinker
StatusCode lights_blinker_init(LightsBlinker *blinker);

// When turned on, blinker will periodically raise events with the sepcified EventID, each time
// alternating the data field, duration is the period between two raised events, and is
// in millisseconds.
StatusCode lights_blinker_on(LightsBlinker *blinker, LightsBlinkerDuration duration, EventID id);

// Blinker off will raise the registered event id with data set to 0, and will cancel the
// scheduled timer
StatusCode lights_blinker_off(LightsBlinker *blinker);

// Resets the blinker, scheduling a new timer, and setting it state to on
StatusCode lights_blinker_reset(LightsBlinker *blinker);

// Returns if the current blinker has active timers
bool lights_blinker_inuse(LightsBlinker *blinker);
