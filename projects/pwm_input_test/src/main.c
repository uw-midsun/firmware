#include "pwm_input.h"
#include "pwm.h"
#include "stm32f0xx.h"
#include "gpio.h"
#include "interrupt.h"
#include "wait.h"

int main(void) {

  uint16_t period = 1000;

  pwm_init(TIM3);
  pwm_set_dc(TIM3, 50);
  gpio_init();

  GPIOAddress output = {
    .port = GPIO_PORT_C, 
    .pin = 9,
  };

  GPIOSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };

  gpio_init_pin(&output, &output_settings);

}
