#include <stddef.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#include "lights_blinker.h"

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  LightsBlinker *blinker = (LightsBlinker *)context;
  blinker->state = !blinker->state;
  event_raise(blinker->event_id, (uint16_t)blinker->state);
  soft_timer_start(blinker->duration_us, prv_timer_callback, (void *)blinker, &blinker->timer_id);
}

// blinker's timer id needs to be initialized to an invalid timer if it's not being used otherwise
// we may have a collision.
StatusCode lights_blinker_init(LightsBlinker *blinker) {
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_on_us(LightsBlinker *blinker, LightsBlinkerDuration duration_us,
                                EventID id) {
  if (lights_blinker_inuse(blinker)) {
    // the passed-in blinker may have active timers
    return STATUS_CODE_INVALID_ARGS;
  }
  blinker->duration_us = duration_us;
  blinker->state = LIGHTS_BLINKER_STATE_ON;
  blinker->event_id = id;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  return soft_timer_start(blinker->duration_us, prv_timer_callback, (void *)blinker,
                          &blinker->timer_id);
}

StatusCode lights_blinker_off(LightsBlinker *blinker) {
  blinker->state = LIGHTS_BLINKER_STATE_OFF;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  // we're expecting this to always return true, since every time the timer goes off, a new timer
  // id gets set.
  if (soft_timer_cancel(blinker->timer_id)) {
    blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_INTERNAL_ERROR;
}

StatusCode lights_blinker_reset(LightsBlinker *blinker) {
  // the passed-in blinker should have an active timer
  if (!lights_blinker_inuse(blinker)) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // cancel its current timer
  soft_timer_cancel(blinker->timer_id);
  blinker->state = LIGHTS_BLINKER_STATE_ON;
  status_ok_or_return(event_raise(blinker->event_id, (uint16_t)blinker->state));
  return soft_timer_start(blinker->duration_us, prv_timer_callback, (void *)blinker,
                          &blinker->timer_id);
}

bool lights_blinker_inuse(LightsBlinker *blinker) {
  return (blinker->timer_id != SOFT_TIMER_INVALID_TIMER);
}

