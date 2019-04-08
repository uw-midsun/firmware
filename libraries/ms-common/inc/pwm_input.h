#pragma once
// PWM Input Module

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

typedef void (*PwmInputHandler)(uint32_t dc, uint32_t period, void *context);

typedef struct {
  PwmInputHandler callback;
  void *context;
} PwmInputStorage;

typedef struct {
  PwmInputHandler callback;
  void *context;
  PwmChannel channel;
} PwmInputSettings;

// Initializes the timer for PWM input.
StatusCode pwm_input_init(PwmTimer timer, PwmInputSettings *settings, PwmInputStorage *storage);

// Gets the period of a PWM input
uint32_t pwm_input_get_period(PwmTimer timer);

// Gets the duty cycle of a PWM input
uint32_t pwm_input_get_dc(PwmTimer timer);
