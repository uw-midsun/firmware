#include "pwm_input.h"
#include "pwm.h"
#include "stm32f0xx.h"
#include "gpio.h"
#include "interrupt.h"
#include "wait.h"

void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);

  IC2Value = TIM_GetCapture2(TIM3);

  if (IC2Value != 0)
  {
    DutyCycle = (TIM_GetCapture1(TIM3) * 100) / IC2Value;

    /* Frequency computation */
    Frequency = SystemCoreClock / IC2Value;
  }
  else
  {
    DutyCycle = 0;
    Frequency = 0;
  }
}

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
    .alt_function = GPIO_AF_2,
    .state = GPIO_STATE_LOW,
  };

  gpio_init_pin(&output, &output_settings);
  pwm_input_init;

}
