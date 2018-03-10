#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_events.h"
#include "lights_gpio.h"
#include "simple_peripherals.h"

static SimplePeripheralCallback prv_simple_callback;

// it registers the callback for all the events that
// correspond to "simple" peripherals,
// that is: horn, headlights, brakes
StatusCode simple_peripherals_init(SimplePeripheralCallback cb) {
  prv_simple_callback = cb;
  return STATUS_CODE_OK;
}

StatusCode simple_peripherals_event(Event e) {
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
