#include "debounce.h"
#include "soft_timer.h"

#define CHECK_INTERVAL_MSEC 5
#define STABLE_INTERVAL_MSEC 50

static bool debounced;
static uint8_t s_num_readings = 0;
static GPIOState debounced_state = 0;

static void prv_debounce(SoftTimerID timer_id, void* context) {
  GPIOAddress* address = context;
  GPIOState current_state = GPIO_STATE_LOW;

  // Read the current pin value and compare to the previous input
  gpio_get_state(address, &current_state);

  if (debounced_state == current_state) {
    s_num_readings++;
    
    if (s_num_readings == STABLE_INTERVAL_MSEC) {
      debounced = true;
      return;
    }
  } else {
    s_num_readings = 0;
    debounced_state = current_state;
  }

  // Continue polling if the input has not been held steady for long enough
  soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, address, &timer_id);
    
}

GPIOState debounce(GPIOAddress address) {
  SoftTimerID timer_id;

  soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, &address, &timer_id);

  while (!debounced) { }

  return debounced_state;
}