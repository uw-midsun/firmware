#pragma once
// PWM Input Module

// Usage Notes:
// - As of the moment, PWM input is hardcoded to TIM1 Channels 2 and 3

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

// Initializes the timer for PWM input.
StatusCode pwm_input_init();

// Handle IRQHandler
StatusCode pwm_input_handle_interrupt();

StatusCode pwm_input_get_period();

StatusCode pwm_input_get_dc();
