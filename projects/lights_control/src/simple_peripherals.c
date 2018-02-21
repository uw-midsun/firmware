#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#include "lights_events.h"
#include "lights_gpio.h"

StatusCode simple_peripherals_init() {
  return STATUS_CODE_OK;
}

StatusCode simple_peripherals_event(Event e) {
  InputEvent event = e.id;
  switch (event) {
    case EVENT_HORN:
    case EVENT_HEADLIGHTS:
    case EVENT_BRAKES:
    case EVENT_STROBE:
      return lights_gpio_set(e);
      break;
    default:
      return STATUS_CODE_OK;
      break;
  }
}
