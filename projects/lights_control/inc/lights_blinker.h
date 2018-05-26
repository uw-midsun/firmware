#pragma once

// lights_blinker aids with the blinking functionality that some lights require such as signals.
// Requires soft_timers and event_queue to be initialized.

// An instance of this module is used to periodically raise LIGHTS_EVENT_GPIO_OFF and
// LIGHTS_EVENT_GPIO_ON events with the data field set to different peripherals. Events are to be
// processed by the lights_gpio module.

#include "event_queue.h"
#include "lights_events.h"
#include "soft_timer.h"

// State definitions for blinker.
typedef enum {
  LIGHTS_BLINKER_STATE_OFF = 0,  // When the corresponding peripheral is OFF
  LIGHTS_BLINKER_STATE_ON,       // When the corresponding peripheral is ON
  NUM_LIGHTS_BLINKER_STATES
} LightsBlinkerState;

typedef uint32_t LightsBlinkerDuration;

// Blinker members.
typedef struct LightsBlinker {
  LightsEventGpioPeripheral peripheral;
  SoftTimerID timer_id;
  LightsBlinkerState state;
  LightsBlinkerDuration duration_ms;  // Duration, in milliseconds
} LightsBlinker;

// Initializes the blinker.
StatusCode lights_blinker_init(LightsBlinker *blinker, LightsBlinkerDuration duration_ms);

// Starts generating LIGHTS_EVENT_GPIO_OFF and LIGHTS_EVENT_GPIO_ON events with data field set to
// the passed-in peripheral. First event generated is an LIGHTS_EVENT_GPIO_ON event.
StatusCode lights_blinker_activate(LightsBlinker *blinker, LightsEventGpioPeripheral peripheral);

// Raises an LIGHTS_EVENT_GPIO_OFF event with the existing blinker's peripheral as event data,
// cancels the scheduled timer. Deactivates blinker.
StatusCode lights_blinker_deactivate(LightsBlinker *blinker);

// Only used for syncing purposes. Reschedules the timer, sets the state to ON.
StatusCode lights_blinker_sync_on(LightsBlinker *blinker);

