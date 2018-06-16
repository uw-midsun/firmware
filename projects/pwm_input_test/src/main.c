#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "pwm_input.h"
#include "stm32f0xx.h"
#include "wait.h"
#include "soft_timer.h"

int main(void) {
  uint16_t period = 1000;

  pwm_init(PWM_TIMER_3, 1000);
  pwm_set_dc(PWM_TIMER_3, 50);
  gpio_init();

  GPIOAddress output = {
    .port = GPIO_PORT_C,
    .pin = 9,
  };

  GPIOSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  gpio_init_pin(&output, &output_settings);

  uint16_t get_period = pwm_get_period(PWM_TIMER_3);

  // pwm_input_init();
}
