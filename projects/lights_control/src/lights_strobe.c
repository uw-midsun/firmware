#include "lights_strobe.h"

StatusCode lights_strobe_init(LightsStrobe *lights_strobe, LightsBlinkerDuration blinker_duration) {
  lights_strobe->state = LIGHTS_STROBE_STATE_OFF;
  StatusCode status = lights_blinker_init(&lights_strobe->blinker, blinker_duration);
  if (!status_ok(status)) {
    return status_msg(STATUS_CODE_UNKNOWN, "Error initializing lights strobe.");
  }
  return STATUS_CODE_OK;
}

StatusCode lights_strobe_process_event(LightsStrobe *lights_strobe, const Event *event) {
  if (event->id == LIGHTS_EVENT_STROBE) {
    LightsStrobeState new_state = (LightsStrobeState)event->data;
    return (new_state) ? lights_blinker_activate(&lights_strobe->blinker, LIGHTS_EVENT_STROBE)
                       : lights_blinker_deactivate(&lights_strobe->blinker);
  }
  return STATUS_CODE_OK;
}
