#pragma once
// PWM Library
//
// Usage Notes:
// - Module must be initialized before use.
// - Use pwm_set_dc unless you have reason not to.
// - pwm_set_pulse affords resolution < 1% of the duty cycle.
// - For stm32f0xx GPIO pins must use the correct GPIO_ALTFN to utilize the PWM.
//
// Implementation Detail:
// - This only enables PWM on channels tied to TIM3 all using the same duty cycle. This may need to
// be altered as not all PWM devices may use this timer. Additionally, each channel (4) of TIM3 may
// want its own pulse width. For the purposes of simplicity this will be left as is for now but may
// need revision in future.

#include <stdint.h>

#include "status.h"

// Initializes the PWM with a period in milliseconds.
StatusCode pwm_init(uint16_t period_ms);

// Gets the current period of the PWM.
uint16_t pwm_get_period(void);

// Sets the pulse width in ms of the PWM. Use for high resolution control.
StatusCode pwm_set_pulse(uint16_t pulse_width_ms);

// Sets the duty cycle, in units of 1%, of the PWM. This wraps the
// pwm_set_pulse doing the necessary math.
StatusCode pwm_set_dc(uint16_t dc);
