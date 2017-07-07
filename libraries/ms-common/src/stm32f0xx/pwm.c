#include "pwm.h"

#include <stdint.h>

#include "gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

static uint16_t s_period_ms = 0;

StatusCode pwm_init(uint16_t period_ms) {
  if (period_ms == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  s_period_ms = period_ms;

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = period_ms,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(TIM3, &tim_init);
  TIM_Cmd(TIM3, ENABLE);
  TIM_CtrlPWMOutputs(TIM3, ENABLE);

  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(void) {
  return s_period_ms;
}

StatusCode pwm_set_pulse(uint16_t pulse_width_ms) {
  if (s_period_ms == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width_ms > s_period_ms) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }

  TIM_OCInitTypeDef oc_init = {
    .TIM_OCMode = TIM_OCMode_PWM1,  // Set on compare match.
    .TIM_Pulse = pulse_width_ms,
    .TIM_OutputState = TIM_OutputState_Enable,
    .TIM_OCPolarity = TIM_OCPolarity_High,
  };

  // Enable PWM on all channels.

  TIM_OC1Init(TIM3, &oc_init);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_OC2Init(TIM3, &oc_init);
  TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_OC3Init(TIM3, &oc_init);
  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

  TIM_OC4Init(TIM3, &oc_init);
  TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(uint16_t dc) {
  if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }

  uint16_t pulse_width = 0;
  if (dc != 0) {
    pulse_width = ((s_period_ms + 1) * dc) / 100 - 1;
  }

  return pwm_set_pulse(pulse_width);
}
