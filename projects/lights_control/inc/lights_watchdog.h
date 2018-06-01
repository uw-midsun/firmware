#pragma once
// Watchdog for BPS heartbeat events.
// Requires soft-timers to be initialized.
#include "event_queue.h"
#include "soft_timer.h"

// When triggered, this module will raise a LIGHTS_GPIO_* event to turn the strobe lights on. It
// will trigger if it doesn't receive a bps heartbeat event after some timeout value, or if the
// event is a BPS_HEARTBEAT_STATE_ERROR.

typedef uint32_t LightsWatchDogTimeout;

typedef struct LightsWatchDogStorage {
  SoftTimerID timer_id;
  LightsWatchDogTimeout timeout_ms;
} LightsWatchDogStorage;

// This initializes the watchdog timer, which will in turn schedule a timer.
StatusCode lights_watchdog_init(LightsWatchDogStorage *storage, LightsWatchDogTimeout timeout_ms);

// Lights watchdog needs to periodically receive a LIGHTS_EVENT_BPS_HEARTBEAT with
// EE_BPS_HEARTBEAT_STATE_OK state. Otherwise it will fire, and turn on strobe lights.
StatusCode lights_watchdog_process_event(LightsWatchDogStorage *storage, const Event *e);
