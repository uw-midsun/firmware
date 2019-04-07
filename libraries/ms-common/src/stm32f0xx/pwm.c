#include "pwm.h"

#include <stdint.h>

#include "pwm_mcu.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

TIM_TypeDef *s_timer_def[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = TIM1,    //
  [PWM_TIMER_3] = TIM3,    //
  [PWM_TIMER_14] = TIM14,  //
  [PWM_TIMER_15] = TIM15,  //
  [PWM_TIMER_16] = TIM16,  //
  [PWM_TIMER_17] = TIM17,  //
};

IRQn_Type s_irq_def[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = TIM1_CC_IRQn,    //
  [PWM_TIMER_3] = TIM3_IRQn,    //
  [PWM_TIMER_14] = TIM14_IRQn,  //
  [PWM_TIMER_15] = TIM15_IRQn,  //
  [PWM_TIMER_16] = TIM16_IRQn,  //
  [PWM_TIMER_17] = TIM17_IRQn,  //
};

void pwm_enable_periph_clock(PwmTimer timer) {
  switch (timer) {
    case PWM_TIMER_1:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
      return;
    case PWM_TIMER_3:
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
      return;
    case PWM_TIMER_14:
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
      return;
    case PWM_TIMER_15:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
      return;
    case PWM_TIMER_16:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
      return;
    case PWM_TIMER_17:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
      return;
    default:
      return;
  }
}

TIM_TypeDef * pwm_timer_to_tim(PwmTimer timer) {
  return s_timer_def[timer];
}

IRQn_Type pwm_timer_to_irq(PwmTimer timer) {
  return s_irq_def[timer];
}
