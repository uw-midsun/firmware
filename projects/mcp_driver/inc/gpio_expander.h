#pragma once

// Requires GPIO to be initialized

#include "gpio.h"

typedef enum {
  GPIO_EXPANDER_PIN_0 = 0,
  GPIO_EXPANDER_PIN_1,
  GPIO_EXPANDER_PIN_2,
  GPIO_EXPANDER_PIN_3,
  GPIO_EXPANDER_PIN_4,
  GPIO_EXPANDER_PIN_5,
  GPIO_EXPANDER_PIN_6,
  GPIO_EXPANDER_PIN_7,
  NUM_GPIO_EXPANDER_PIN
} GPIOExpanderPin;

typedef void (*GPIOExpanderCallback)(GPIOExpanderPin pin, void *context);

// Initialize the expander and configure the given address to receive interrupts
StatusCode gpio_expander_init(GPIOAddress address);

StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings);
