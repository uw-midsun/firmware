#pragma once
// PWM Input Module

// Usage Notes:
// - As of the moment, PWM input is hardcoded to TIM1 Channels 2 and 3

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"
#include "pwm.h"

// Initializes the timer for PWM input.
StatusCode pwm_input_init(PwmTimer timer);

// Handle IRQHandler
StatusCode pwm_input_handle_interrupt(PwmTimer timer);

uint32_t pwm_input_get_period();

uint32_t pwm_input_get_dc();
