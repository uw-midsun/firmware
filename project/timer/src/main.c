#include <stdbool.h>
#include "log.h"
#include "timer.h"
#include "gpio.h"

static int s_time = 15;

static void prv_timeout_cb(SoftTimerID timer_id, void *context) {
  GPIOAddress *led = context;
  gpio_toggle_state(&led);

  timer_start(s_time, prv_timeout_cb, led, NULL);
}

int main(void) {
  LOG_DEBUG("hello\n");

  gpio_init();

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };
  GPIOAddress s_led = { GPIO_PORT_A, 0 };

  gpio_init_pin(&s_led, &led_settings);
  timer_init();

  uint32_t counter = 0;

  timer_start(s_time, prv_timeout_cb, &s_led, NULL);

  while (true);

  return 0;
}
