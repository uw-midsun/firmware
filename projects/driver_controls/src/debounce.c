#include "debounce.h"

#define CHECK_INTERVAL_MSEC 5
#define STABLE_INTERVAL_MSEC 50

static uint8_t s_num_readings = 0;
static uint8_t debounced_state = 0;

static void prv_debounce(SoftTimerID timer_id, void* context) {
  gpio_get_state(const GPIOAddress *address, GPIOState *input_state);
}

StatusCode debounce(GPIOAddress address) {
  SoftTimerID timer_id;

  GPIOState debounced_state = GPIO_STATE_HIGH;

  soft_timer_start(CHECK_INTERVAL_MSEC, prv_debounce, &debounced_state, *timer_id);
  return 0;
}