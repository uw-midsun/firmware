#pragma once
// PWM Input Module

#include <stdint.h>

#include "pwm_mcu.h"
#include "status.h"

typedef void (*PwmInputHandler)(uint32_t dc, uint32_t period, void *context);

// Don't worry about initializing this struct. Can just pass in an empty one
// as long as memory has been allocated for it. The PWM driver will use it as storage
typedef struct {
  PwmInputHandler callback;
  void *context;
} PwmInputStorage;

//
// Settings struct
//

// This settings struct can have a callback which will be called
// whenever a new PWM reading is read UNLESS the DC value is 0. This
// is because the timer interrupt does not trigger for values of 0 (there is no rising edge).
// As a result, it is advised that the user also calls |pwm_input_get_dc| sometimes as this
// will force the callback to fire for DC values of 0.

// As well, there is a channel parameter. It can only be a value of one or two. This is
// because the |TIM_PWMIConfig| only works on those two channels.
typedef struct {
  PwmInputHandler callback;
  void *context;
  PwmChannel channel;
} PwmInputSettings;

// Initializes the timer for PWM input. In order to choose a timer, and a timer channel
// for a GPIO pin, please consult:
//
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/16253071/Resources?preview=/16253071/38486554/stm32f072_af.xlsx
StatusCode pwm_input_init(PwmTimer timer, PwmInputSettings *settings, PwmInputStorage *storage);

// Gets the period of a PWM input in us.
uint32_t pwm_input_get_period(PwmTimer timer);

// Gets the duty cycle of a PWM input. The result should be a number in the range
// [0, 1000]. The result represents a percentage of 0-100% with one decimal place
uint32_t pwm_input_get_dc(PwmTimer timer);
