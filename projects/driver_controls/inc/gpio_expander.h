#pragma once

// Driver for the MCP23008 GPIO Expander
// Requires GPIO, Interrupts, and I2C to be initialized

#include <stdbool.h>

#include "gpio.h"
#include "i2c.h"

typedef enum {
  GPIO_EXPANDER_PIN_0 = 0,
  GPIO_EXPANDER_PIN_1,
  GPIO_EXPANDER_PIN_2,
  GPIO_EXPANDER_PIN_3,
  GPIO_EXPANDER_PIN_4,
  GPIO_EXPANDER_PIN_5,
  GPIO_EXPANDER_PIN_6,
  GPIO_EXPANDER_PIN_7,
  NUM_GPIO_EXPANDER_PINS
} GPIOExpanderPin;

typedef void (*GPIOExpanderCallback)(GPIOExpanderPin pin, GPIOState state, void *context);

// Initialize the expander with an address to connect to its INT pin
StatusCode gpio_expander_init(GPIOAddress address, I2CPort i2c_port);

// Initialize one of the expander pins.
StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings);

// Returns the set output value or the current input value based on the pin
// configuration
StatusCode gpio_expander_get_state(GPIOExpanderPin pin, GPIOState *state);

// Set the state of an output pin
StatusCode gpio_expander_set_state(GPIOExpanderPin pin, GPIOState new_state);

// Register a callback for a specific pin
StatusCode gpio_expander_register_callback(GPIOExpanderPin pin, GPIOExpanderCallback callback,
                                           void *context);
