#include "debounce.h"

#define CHECK_INTERVAL_MSEC 5
#define STABLE_INTERVAL_MSEC 50

static uint8_t s_num_readings = 0;
static GPIOState debounced_state = 0;

static void prv_debounce(SoftTimerID timer_id, void* context) {
  GPIOAddress* address = context;
  GPIOState current_state = GPIO_STATE_LOW;

  gpio_get_state(address, current_state);

  if (debounced_state == current_state) {
    s_num_readings ++;
  } else {
    s_num_readings = 0;
    debounced_state = current_state;
  }
}

GPIOState debounce(GPIOAddress address) {
  SoftTimerID timer_id;

  while (s_num_readings < (STABLE_INTERVAL_MSEC/CHECK_INTERVAL_MSEC)) {
    soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, &address, *timer_id);
    while (soft_timer_inuse()) {}
  }

  return debounced_state;
}