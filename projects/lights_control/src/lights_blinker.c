#include <stddef.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#include "lights_blinker.h"

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  LightsBlinker *blinker = (LightsBlinker *)context;
  blinker->state = !blinker->state;
  event_raise(blinker->event_id, (uint16_t)blinker->state);
  soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                          &blinker->timer_id);
}

// Blinker's timer id needs to be initialized to an invalid timer if it's not being used otherwise
// we may have a collision.
StatusCode lights_blinker_init(LightsBlinker *blinker) {
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_on(LightsBlinker *blinker, LightsBlinkerDuration duration_ms,
                             EventID id) {
  if (lights_blinker_inuse(blinker)) {
    // The passed-in blinker may have active timers.
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "Can't call lights_blinker_on on an already ON blinker.");
  }
  blinker->duration_ms = duration_ms;
  blinker->state = LIGHTS_BLINKER_STATE_ON;
  blinker->event_id = id;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                 &blinker->timer_id);
}

StatusCode lights_blinker_off(LightsBlinker *blinker) {
  if (blinker->timer_id == SOFT_TIMER_INVALID_TIMER) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "An already off blinker cannot be turned off again.");
  }
  blinker->state = LIGHTS_BLINKER_STATE_OFF;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  // Cancel the scheduled timer.
  soft_timer_cancel(blinker->timer_id);
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_reset(LightsBlinker *blinker) {
  // The passed in blinker should have an active timer.
  if (!lights_blinker_inuse(blinker)) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // Cancel its current timer.
  soft_timer_cancel(blinker->timer_id);
  blinker->state = LIGHTS_BLINKER_STATE_ON;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                 &blinker->timer_id);
}

bool lights_blinker_inuse(LightsBlinker *blinker) {
  return (blinker->timer_id != SOFT_TIMER_INVALID_TIMER);
}
