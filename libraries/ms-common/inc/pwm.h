#pragma once

#include "pwm_mcu.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

void pwm_enable_periph_clock(PwmTimer timer);

TIM_TypeDef * pwm_timer_to_tim(PwmTimer timer);

IRQn_Type pwm_timer_to_irq(PwmTimer timer);
