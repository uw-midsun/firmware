#include "lights_strobe.h"

StatusCode lights_strobe_init(LightsStrobeStorage *lights_strobe,
                              LightsBlinkerDuration blinker_duration) {
  return lights_blinker_init(&lights_strobe->blinker, blinker_duration,
                             LIGHTS_BLINKER_COUNT_THRESHOLD_NO_SYNC);
}

StatusCode lights_strobe_process_event(LightsStrobeStorage *lights_strobe, const Event *event) {
  if (event->id == LIGHTS_EVENT_STROBE_ON) {
    status_ok_or_return(
        lights_blinker_activate(&lights_strobe->blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE));
  } else if (event->id == LIGHTS_EVENT_STROBE_OFF) {
    status_ok_or_return(lights_blinker_deactivate(&lights_strobe->blinker));
  }
  return STATUS_CODE_OK;
}
