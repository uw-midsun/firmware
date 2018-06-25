#include "pwm_input.h"

#include <stdint.h>

#include "gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_interrupt.h"


// This is probably not up to par with coding standards
// Will change once functionality is confirmed
static uint32_t period = 0;
static uint32_t dc = 0;

StatusCode pwm_input_init() {
  stm32f0xx_interrupt_init();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  stm32f0xx_interrupt_nvic_enable(TIM1_CC_IRQn, 0);

  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = TIM_Channel_2,
    .TIM_ICPolarity = TIM_ICPolarity_Rising,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };
  
  TIM_PWMIConfig(TIM1, &tim_icinit);
  TIM_SelectInputTrigger(TIM1, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);
  TIM_Cmd(TIM1, ENABLE);
  TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode pwm_input_handle_interrupt() {
  TIM_ClearITPendingBit(TIM1, TIM_IT_CC2);
  uint32_t IC2Value = TIM_GetCapture2(TIM1);

  if (IC2Value != 0) {
    dc = (TIM_GetCapture1(TIM2) * 100) / IC2Value;
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
