#include "pwm_input.h"

#include <stdint.h>

#include "pwm.h"
#include "gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_interrupt.h"


// This is probably not up to par with coding standards
// Will change once functionality is confirmed
static volatile uint32_t period = 0;
static volatile uint32_t dc = 0;

static const IRQn_Type timer_to_irq [NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = TIM1_CC_IRQn,   //
  [PWM_TIMER_3] = TIM3_IRQn,   //
  [PWM_TIMER_14] = 0,  //
  [PWM_TIMER_15] = 0,  //
  [PWM_TIMER_16] = 0,  //
  [PWM_TIMER_17] = 0,  //
};

StatusCode pwm_input_init(PwmTimer timer) {
  // TIM1
  pwm_enable_periph_clock(timer);

  // TIM1_CC_IRQn
  stm32f0xx_interrupt_nvic_enable(timer_to_irq[timer], 0);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = 0xFFFFFFFF,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(timer_def[timer], &tim_init);

  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = TIM_Channel_1,
    .TIM_ICPolarity = TIM_ICPolarity_Falling,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };

  TIM_PWMIConfig(timer_def[timer], &tim_icinit);

  TIM_SelectInputTrigger(timer_def[timer], TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(timer_def[timer], TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(timer_def[timer], TIM_MasterSlaveMode_Enable);
  TIM_SelectOutputTrigger(timer_def[timer], TIM_TRGOSource_Reset);
  TIM_Cmd(timer_def[timer], ENABLE);
  TIM_ITConfig(timer_def[timer], TIM_IT_CC1, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode pwm_input_handle_interrupt(PwmTimer timer) {

  TIM_ClearITPendingBit(timer_def[timer], TIM_IT_CC1);

  uint32_t IC2Value = TIM_GetCapture2(timer_def[timer]);

  if (IC2Value != 0) {
    uint32_t IC2Value_2 = TIM_GetCapture1(timer_def[timer]);
    dc = (IC2Value_2 * 100) / IC2Value;

    // Not sure how to calculate period. Requires time as well
    period = IC2Value;
  } else {
    dc = 0;
    period = 0;
  }

  return STATUS_CODE_OK;
}

uint32_t pwm_input_get_period() {
  return period;
}

uint32_t pwm_input_get_dc() {
  return dc;
}
