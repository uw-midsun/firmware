#include "lights_blinker.h"
#include "lights_events.h"

static LightsEvent s_blinker_event = NUM_LIGHTS_EVENTS;

// Mocking lights_blinker's behaviour.
StatusCode lights_blinker_init(LightsBlinker *blinker, LightsBlinkerDuration duration_ms) {
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_activate(LightsBlinker *blinker, EventID id) {
  LightsEvent blink_event;
  if (s_blinker_event != id) {
    if (s_blinker_event != NUM_LIGHTS_EVENTS) {
      // If the blinker is already active, cancel the old one.
      status_ok_or_return(lights_events_get_blink_off_event(s_blinker_event, &blink_event));
      status_ok_or_return(event_raise(blink_event, LIGHTS_BLINKER_STATE_OFF));
    }
    s_blinker_event = id;
    status_ok_or_return(lights_events_get_blink_on_event(s_blinker_event, &blink_event));
    return event_raise(blink_event, LIGHTS_BLINKER_STATE_ON);
  }
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_deactivate(LightsBlinker *blinker) {
  LightsEvent blink_event = 0;
  status_ok_or_return(lights_events_get_blink_off_event(s_blinker_event, &blink_event));
  event_raise(blink_event, LIGHTS_BLINKER_STATE_OFF);
  s_blinker_event = NUM_LIGHTS_EVENTS;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_sync_on(LightsBlinker *blinker) {
  return STATUS_CODE_OK;
}
