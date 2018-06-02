#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "pwm_input.h"
#include "stm32f0xx.h"
#include "wait.h"

uint16_t TIM3_IRQHandler(void) {
  TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

  uint16_t IC2Value = TIM_GetCapture2(TIM3);

  if (IC2Value != 0) {
    return (TIM_GetCapture1(TIM3) * 100) / IC2Value;

    // Frequency = SystemCoreClock / IC2Value;
  } else {
    return 0;
  }
}

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
    .alt_function = GPIO_AF_2,
    .state = GPIO_STATE_LOW,
  };

  gpio_init_pin(&output, &output_settings);
  pwm_input_init();
}
