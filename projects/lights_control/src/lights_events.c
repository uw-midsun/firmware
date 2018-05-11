#include "lights_events.h"
#include <stdbool.h>

static bool s_has_blink(LightsEvent event) {
  return (event <= LIGHTS_EVENT_STROBE) && (LIGHTS_EVENT_SIGNAL_HAZARD <= event);
}

StatusCode lights_events_get_blink_on_event(LightsEvent event, LightsEvent *blink_event) {
  if (!s_has_blink(event)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Event does not have a corresponding blink event.");
  }
  *blink_event = event - LIGHTS_EVENTS_BLINK_ON_OFFSET;
  return STATUS_CODE_OK;
}

StatusCode lights_events_get_blink_off_event(LightsEvent event, LightsEvent *blink_event) {
  if (!s_has_blink(event)) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Event does not have a corresponding blink event.");
  }
  *blink_event = event - LIGHTS_EVENTS_BLINK_OFF_OFFSET;
  return STATUS_CODE_OK;
}

