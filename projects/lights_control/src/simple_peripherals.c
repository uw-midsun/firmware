#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_events.h"
#include "simple_peripherals.h"

static SimplePeripheralCallback prv_simple_callback;

StatusCode lights_simple_peripherals_init(SimplePeripheralCallback cb) {
  prv_simple_callback = cb;
  return STATUS_CODE_OK;
}

StatusCode lights_simple_peripherals_process_event(Event e) {
  InputEvent event = e.id;
  switch (event) {
    case EVENT_HORN:
    case EVENT_HEADLIGHTS:
    case EVENT_BRAKES:
      return prv_simple_callback(e);
      break;
    default:
      return STATUS_CODE_OK;
      break;
  }
}
