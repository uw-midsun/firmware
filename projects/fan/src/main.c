#include <stdbool.h>

#include "gpio.h"
#include "pwm.h"

int main(void) {
  gpio_init();
  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,
    .resistor = GPIO_RES_NONE,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_2,
  };
  GPIOAddress addr = {
    .port = GPIO_PORT_A,
    .pin = 9,
  };
  gpio_init_pin(&addr, &settings);
  addr.pin = 10;
  gpio_init_pin(&addr, &settings);
  pwm_init(PWM_TIMER_1, 5);
  pwm_set_dc(PWM_TIMER_1, 70);
  while (true) {
  }
}
