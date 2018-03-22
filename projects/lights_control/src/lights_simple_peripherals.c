#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_events.h"
#include "lights_simple_peripherals.h"

static SimplePeripheralCallback prv_simple_callback;

StatusCode lights_simple_peripherals_init(SimplePeripheralCallback cb) {
  prv_simple_callback = cb;
  return STATUS_CODE_OK;
}

StatusCode lights_simple_peripherals_process_event(Event e) {
  switch (e.id) {
    case LIGHTS_EVENT_HORN:
    case LIGHTS_EVENT_HIGH_BEAMS:
    case LIGHTS_EVENT_LOW_BEAMS:
    case LIGHTS_EVENT_DRL:
    case LIGHTS_EVENT_BRAKES:
      return prv_simple_callback(e);
      break;
    default:
      return STATUS_CODE_OK;
      break;
  }
}
