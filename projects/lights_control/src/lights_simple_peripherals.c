#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_events.h"
#include "lights_simple_peripherals.h"

static LightsSimplePeripheralCallback s_simple_callback;

StatusCode lights_simple_peripherals_init(LightsSimplePeripheralCallback cb) {
  s_simple_callback = cb;
  return STATUS_CODE_OK;
}
static LightsEvent s_supported_events[] = {
  LIGHTS_EVENT_HORN, LIGHTS_EVENT_HIGH_BEAMS, LIGHTS_EVENT_LOW_BEAMS,
  LIGHTS_EVENT_DRL,  LIGHTS_EVENT_BRAKES,
};

bool prv_is_supported(const Event *e) {
  for (uint8_t i = 0; i < SIZEOF_ARRAY(s_supported_events); i++) {
    if (s_supported_events[i] == e->id) {
      return true;
    }
  }
  return false;
}

StatusCode lights_simple_peripherals_process_event(const Event *e) {
  if (prv_is_supported(e)) {
    return s_simple_callback(e);
  }
  return STATUS_CODE_OK;
}
