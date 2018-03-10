#include "status.h"
#include "event_queue.h"
#include "gpio.h"

#include "lights_events.h"
#include "blinker.h"
#include "strobe.h"

static Blinker s_blinker;

static BlinkerDuration s_duration_millis;

static StrobeCallback prv_strobe_callback;

void prv_strobe_blinker_cb(BlinkerState s) {
  Event event = {
    .id = EVENT_STROBE, //
    .data = (StrobeState) s //
  };
  prv_strobe_callback(event);
}

StatusCode strobe_init(StrobeCallback cb, BlinkerDuration duration) {
  prv_strobe_callback = cb;
  s_duration_millis = duration;
  blinker_init(&s_blinker, prv_strobe_blinker_cb);
  return STATUS_CODE_OK;
}

StatusCode strobe_event_process(Event e) {
  if (e.id == EVENT_STROBE) {
    if (e.data) {
      blinker_on_millis(&s_blinker, s_duration_millis);
    } else {
      blinker_off(&s_blinker);
    }
  }
  return STATUS_CODE_OK;
}

