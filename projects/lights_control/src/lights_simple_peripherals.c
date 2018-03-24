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

StatusCode lights_simple_peripherals_process_event(const Event *e) {
  if (e->id <= LIGHTS_EVENT_BRAKES) {
    return s_simple_callback(e);
  }
  return STATUS_CODE_OK;
}
