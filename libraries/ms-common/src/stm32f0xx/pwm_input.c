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

StatusCode pwm_input_init() {
  // TIM1
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM3, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // TIM1_CC_IRQn
  stm32f0xx_interrupt_nvic_enable(TIM3_IRQn, 0);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = 0xFFFFFFFF,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(TIM3, &tim_init);

  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = TIM_Channel_1,
    .TIM_ICPolarity = TIM_ICPolarity_Rising,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };

  TIM_PWMIConfig(TIM3, &tim_icinit);

  TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);
  TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Reset);
  TIM_Cmd(TIM3, ENABLE);
  TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode pwm_input_handle_interrupt() {
  TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

  uint32_t IC2Value = TIM_GetCapture1(TIM3);

  if (IC2Value != 0) {
    uint32_t IC2Value_2 = TIM_GetCapture2(TIM3);
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
