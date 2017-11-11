#include <stdbool.h>

#include "debounce.h"
#include "soft_timer.h"

#define CHECK_INTERVAL_MSEC 5
#define STABLE_INTERVAL_MSEC 50

static volatile bool s_debounced;
static uint8_t s_num_readings;
static GPIOState s_debounced_state;

static void prv_debounce(SoftTimerID timer_id, void* context) {
  GPIOAddress* address = context;

  // Read the current pin value and compare to the previous input
  GPIOState current_state;
  gpio_get_state(address, &current_state);

  if (s_debounced_state == current_state) {
    s_num_readings++;

    if (s_num_readings == STABLE_INTERVAL_MSEC) {
      s_debounced = true;
      return;
    }
  } else {
    s_num_readings = 0;
  }

  // Continue polling if the input has not been held steady for long enough
  s_debounced_state = current_state;
  soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, address, &timer_id);
}

GPIOState debounce(GPIOAddress address) {
  s_debounced = false;
  s_num_readings = 0;
  s_debounced_state = GPIO_STATE_LOW;

  SoftTimerID timer_id;

  soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, &address, &timer_id);

  while (!s_debounced) {
  }

  return s_debounced_state;
}
