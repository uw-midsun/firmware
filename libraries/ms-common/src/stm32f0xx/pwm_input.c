#include "pwm_input.h"

#include <stdint.h>

#include "gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

StatusCode pwm_input_init() {

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

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