#pragma once
// PWM Input Module

// Usage Notes:
// - As of the moment, PWM input is hardcoded to TIM2

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

// Initializes the timer for PWM input.
StatusCode pwm_init();
