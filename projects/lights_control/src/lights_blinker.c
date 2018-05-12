#include <stddef.h>
#include <stdio.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"

// Switches the blinker state, raises a LIGHTS_EVENT_GPIO_OFF or LIGHTS_EVENT_GPIO_ON event with
// the appropriate peripheral, and schedules a new timer.
static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  LightsBlinker *blinker = (LightsBlinker *)context;
  blinker->state = !blinker->state;
  // Choose whether to raise an ON or OFF event.
  LightsEvent event =
      (blinker->state == LIGHTS_BLINKER_STATE_OFF) ? LIGHTS_EVENT_GPIO_OFF : LIGHTS_EVENT_GPIO_ON;
  event_raise(event, blinker->peripheral);
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

StatusCode lights_blinker_activate(LightsBlinker *blinker, LightsEventGPIOPeripheral peripheral) {
  // Check if we require a behaviour change. i.e. new event is for a different peripheral.
  // e.g. If signal-left and hazard are both on at the same time, turning signal-left off shouldn't
  // do anything.
  if (blinker->peripheral != peripheral) {
    if (prv_lights_blinker_is_active(blinker)) {
      // Previously active, cancel the old one.
      status_ok_or_return(lights_blinker_deactivate(blinker));
    }
    blinker->peripheral = peripheral;
    blinker->state = LIGHTS_BLINKER_STATE_ON;
    status_ok_or_return(event_raise(LIGHTS_EVENT_GPIO_ON, blinker->peripheral));
    return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                   &blinker->timer_id);
  }
  // No behaviour change required. Do nothing.
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_deactivate(LightsBlinker *blinker) {
  if (!prv_lights_blinker_is_active(blinker)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Blinker already inactive");
  }
  soft_timer_cancel(blinker->timer_id);
  blinker->state = LIGHTS_BLINKER_STATE_OFF;
  // Cancel the scheduled timer.
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  status_ok_or_return(event_raise(LIGHTS_EVENT_GPIO_OFF, blinker->peripheral));
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
  status_ok_or_return(event_raise(LIGHTS_EVENT_GPIO_ON, blinker->peripheral));
  return soft_timer_start_millis(blinker->duration_ms, prv_timer_callback, (void *)blinker,
                                 &blinker->timer_id);
}
