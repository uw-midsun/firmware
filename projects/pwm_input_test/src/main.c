#include "gpio.h"
#include "pwm.h"
#include "pwm_input.h"

int main(void) {

  // Set a PWM signal of 1000ms with a duty cycle of 50%
  // Should blink for half a second
  pwm_init(PWM_TIMER_3, 1000);
  pwm_set_dc(PWM_TIMER_3, 50);
  gpio_init();

// Use port for Green LED
  GPIOAddress output = {
    .port = GPIO_PORT_C,
    .pin = 9,
  };

  GPIOSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_0
  };

  gpio_init_pin(&output, &output_settings);

  // TODO: Initialize PWM input on another pin and test using jumper wire
  // Use TIM1_CH2 to use pin PA9

  // Pray this works
  for (;;) {
  }

}
