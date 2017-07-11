#include "debounce.h"

#include <stdio.h>
#include "soft_timer.h"

#define CHECK_MSEC    2       // Sampling interval in milliseconds
#define HOLD_MSEC     50      // Hold time for button presses

static void prv_output(SoftTimerID timer_id, void *context) {
  return;
}

void debounce(GPIOAddress *address, GPIOState *current_state) {
  GPIOState prev_state = *current_state;

  SoftTimerID timer_id;

  uint32_t count = HOLD_MSEC / CHECK_MSEC;

  for (uint32_t i = 0; i < count; i++) {
    soft_timer_start_millis(CHECK_MSEC, prv_output, NULL, &timer_id);
    while (soft_timer_inuse()) { }
    soft_timer_cancel(timer_id);

    gpio_get_value(address, current_state);

    if (*current_state != prev_state) {
      i = 0;
      prev_state = *current_state;
    }
  }

  return STATUS_CODE_OK;
}
