#include "debounce.h"

#include <stdio.h>
#include "soft_timer.h"

static void prv_output(SoftTimerID timer_id, void *context) {
  return;
}

void debounce(GPIOAddress *address, GPIOState *key_pressed) {
  uint8_t count = (*key_pressed) ?
                   (PRESS_MSEC / CHECK_MSEC) :
                   (RELEASE_MSEC / CHECK_MSEC);

  GPIOState prev_state = 0;
  SoftTimerID timer_a;

  for (uint8_t i = 0; i < count; i++) {
    soft_timer_start(CHECK_MSEC*1000, prv_output, 0, &timer_a);

    while (soft_timer_remaining_time(timer_a) > 0) {}
    soft_timer_cancel(timer_a);

    if (*key_pressed != prev_state) {
      i = 0;
    }

    prev_state = *key_pressed;
  }
}
