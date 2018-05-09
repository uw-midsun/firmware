#include <stddef.h>
#include <stdio.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"

// Dummy data since we have BLINK_ON and BLINK_OFF states
#define LIGHTS_BLINKER_DUMMY_DATA 0

// Utility function to get the corresponding blink event of an event, and raise it.
static StatusCode prv_raise_blink_event(LightsEvent event, LightsBlinkerState state) {
  LightsEvent blink_event = 0;
  if (state == LIGHTS_BLINKER_STATE_ON) {
    status_ok_or_return(lights_events_get_blink_on_event(event, &blink_event));
  } else {
    status_ok_or_return(lights_events_get_blink_off_event(event, &blink_event));
  }
  return event_raise(blink_event, LIGHTS_BLINKER_DUMMY_DATA);
}

// Switches the state, raises blink event, and schedules a new timer.
static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  LightsBlinker *blinker = (LightsBlinker *)context;
  blinker->state = !blinker->state;
  prv_raise_blink_event(blinker->event_id, blinker->state);
  soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                          &blinker->timer_id);
}

// Utility function to see if blinker is already active.
static bool prv_lights_blinker_is_active(LightsBlinker *blinker) {
  return (blinker->timer_id != SOFT_TIMER_INVALID_TIMER);
}

// Blinker's timer id needs to be initialized to an invalid timer if it's not being used otherwise
// we may have a collision.
StatusCode lights_blinker_init(LightsBlinker *blinker, LightsBlinkerDuration duration_ms) {
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  blinker->duration_ms = duration_ms;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_activate(LightsBlinker *blinker, EventID id) {
  // Check if a new event is passed.
  if (blinker->event_id != id) {
    if (prv_lights_blinker_is_active(blinker)) {
      // Previously active, cancel the old one.
      status_ok_or_return(lights_blinker_deactivate(blinker));
    }
    blinker->event_id = id;
    blinker->state = LIGHTS_BLINKER_STATE_ON;
    status_ok_or_return(prv_raise_blink_event(id, LIGHTS_BLINKER_STATE_ON));
    return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                   &blinker->timer_id);
  }
  // If the event passed is same as the old one, do nothing.
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_deactivate(LightsBlinker *blinker) {
  if (!prv_lights_blinker_is_active(blinker)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Blinker already inactive");
  }
  blinker->state = LIGHTS_BLINKER_STATE_OFF;
  // Cancel the scheduled timer.
  soft_timer_cancel(blinker->timer_id);
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  status_ok_or_return(prv_raise_blink_event(blinker->event_id, LIGHTS_BLINKER_STATE_OFF));
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_sync_on(LightsBlinker *blinker) {
  // The passed in blinker should have an active timer.
  if (!prv_lights_blinker_is_active(blinker)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Can't sync a deactivated blinker.");
  }
  // Cancel its current timer.
  soft_timer_cancel(blinker->timer_id);
  blinker->state = LIGHTS_BLINKER_STATE_ON;
  status_ok_or_return(prv_raise_blink_event(blinker->event_id, LIGHTS_BLINKER_STATE_ON));
  return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                 &blinker->timer_id);
}
