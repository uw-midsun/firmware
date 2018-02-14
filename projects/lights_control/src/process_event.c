#include "event_queue.h"
#include "fsm.h"
#include "lights_events.h"
#include "lights_periph.h"
#include "status.h"

static FSM s_signal_fsm_front;

StatusCode process_event(Event e) {
  InputEvent event = e.id;
  switch (event) {
    case EVENT_HORN:
    case EVENT_HEADLIGHTS:
    case EVENT_BRAKES:
    case EVENT_STROBE:
      return lights_periph_simple(e);
      break;
    default:
      return STATUS_CODE_OK;
      break;
  }
}
