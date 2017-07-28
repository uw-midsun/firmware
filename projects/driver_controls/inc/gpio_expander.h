#pragma once

// Requires GPIO and Interrupts to be initialized

#include <stdbool.h>

#include "gpio.h"
#include "interrupt.h"

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

// Initialize one of the expander's pins.
StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings);

// Obtain the current state of the pin
bool gpio_expander_get_state(GPIOExpanderPin pin);

// Set the current state of the pin
StatusCode gpio_expander_set_state(GPIOExpanderPin pin, bool new_state);

// Register a callback
StatusCode gpio_expander_register_callback(GPIOExpanderPin pin,
                                           GPIOExpanderCallback callback, void *context);
