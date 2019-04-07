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

static StatusCode prv_input_handle_interrupt(PwmTimer timer) {
  
  TIM_TypeDef * tim_location = pwm_timer_to_tim(timer);

  TIM_ClearITPendingBit(tim_location, TIM_IT_CC1);

  uint32_t IC2Value = TIM_GetCapture2(tim_location);

  if (IC2Value != 0) {
    uint32_t IC2Value_2 = TIM_GetCapture1(tim_location);
    dc = (IC2Value_2 * 100) / IC2Value;

    period = IC2Value;
  } else {
    dc = 0;
    period = 0;
  }

  return STATUS_CODE_OK;
}

void TIM1_CC_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_1);
}

void TIM3_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_3);
}

StatusCode pwm_input_init(PwmTimer timer) {
  pwm_enable_periph_clock(timer);

  stm32f0xx_interrupt_nvic_enable(pwm_timer_to_irq(timer), 0);

  TIM_TypeDef * tim_location = pwm_timer_to_tim(timer);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = 0xFFFFFFFF,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(tim_location, &tim_init);

  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = TIM_Channel_2,
    .TIM_ICPolarity = TIM_ICPolarity_Rising,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };

  TIM_PWMIConfig(tim_location, &tim_icinit);

  TIM_SelectInputTrigger(tim_location, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(tim_location, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(tim_location, TIM_MasterSlaveMode_Enable);
  TIM_SelectOutputTrigger(tim_location, TIM_TRGOSource_Reset);
  TIM_Cmd(tim_location, ENABLE);
  TIM_ITConfig(tim_location, TIM_IT_CC1, ENABLE);

  return STATUS_CODE_OK;
}

uint32_t pwm_input_get_period() {
  return period;
}

uint32_t pwm_input_get_dc() {
  return dc;
}
