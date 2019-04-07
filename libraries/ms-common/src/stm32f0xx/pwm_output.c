#include "pwm_output.h"

#include <stdint.h>

#include "pwm.h"
#include "gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

static uint16_t s_period_us[NUM_PWM_TIMERS] = {
  [PWM_TIMER_1] = 0,   //
  [PWM_TIMER_3] = 0,   //
  [PWM_TIMER_14] = 0,  //
  [PWM_TIMER_15] = 0,  //
  [PWM_TIMER_16] = 0,  //
  [PWM_TIMER_17] = 0,  //
};

StatusCode pwm_init(PwmTimer timer, uint16_t period_us) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (period_us == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  pwm_enable_periph_clock(timer);

  s_period_us[timer] = period_us;

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = period_us,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(pwm_timer_to_tim(timer), &tim_init);
  TIM_Cmd(pwm_timer_to_tim(timer), ENABLE);
  TIM_CtrlPWMOutputs(pwm_timer_to_tim(timer), ENABLE);

  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(PwmTimer timer) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  }
  return s_period_us[timer];
}

StatusCode pwm_set_pulse(PwmTimer timer, uint16_t pulse_width_us) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (s_period_us[timer] == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width_us > s_period_us[timer]) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }

  TIM_OCInitTypeDef oc_init = {
    .TIM_OCMode = TIM_OCMode_PWM1,  // Set on compare match.
    .TIM_Pulse = pulse_width_us,
    .TIM_OutputState = TIM_OutputState_Enable,
    .TIM_OCPolarity = TIM_OCPolarity_High,
  };

  // Enable PWM on all channels.

  TIM_OC1Init(pwm_timer_to_tim(timer), &oc_init);
  TIM_OC1PreloadConfig(pwm_timer_to_tim(timer), TIM_OCPreload_Enable);

  TIM_OC2Init(pwm_timer_to_tim(timer), &oc_init);
  TIM_OC2PreloadConfig(pwm_timer_to_tim(timer), TIM_OCPreload_Enable);

  TIM_OC3Init(pwm_timer_to_tim(timer), &oc_init);
  TIM_OC3PreloadConfig(pwm_timer_to_tim(timer), TIM_OCPreload_Enable);

  TIM_OC4Init(pwm_timer_to_tim(timer), &oc_init);
  TIM_OC4PreloadConfig(pwm_timer_to_tim(timer), TIM_OCPreload_Enable);

  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(PwmTimer timer, uint16_t dc) {
  if (timer >= NUM_PWM_TIMERS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid timer id");
  } else if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }

  uint16_t pulse_width = 0;
  if (dc != 0) {
    pulse_width = ((s_period_us[timer] + 1) * dc) / 100 - 1;
    if (pulse_width == 0) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle is not valid for given period");
    }
  }

  return pwm_set_pulse(timer, pulse_width);
}
