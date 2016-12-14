#pragma once
// GPIO HAL Interface
#include <stdint.h>

#include "status.h"

// GPIO address to be used to change that pin's settings
typedef struct GPIOAddress {
  uint8_t port;
  uint8_t pin;
} GPIOAddress;

// For setting the direction of the pin
typedef enum {
  GPIO_DIR_IN = 0,
  GPIO_DIR_OUT,
  NUM_GPIO_DIR,
} GPIODir;

// For setting the output value of the pin
typedef enum {
  GPIO_STATE_LOW = 0,
  GPIO_STATE_HIGH,
  NUM_GPIO_STATE,
} GPIOState;

// For setting the internal pull-up/pull-down resistor
typedef enum {
  GPIO_RES_NONE = 0,
  GPIO_RES_PULLUP,
  GPIO_RES_PULLDOWN,
  NUM_GPIO_RES,
} GPIORes;

// For setting the alternate function on the pin
typedef enum {
  GPIO_ALTFN_NONE = 0,
  GPIO_ALTFN_0,
  GPIO_ALTFN_1,
  GPIO_ALTFN_2,
  GPIO_ALTFN_3,
  GPIO_ALTFN_4,
  GPIO_ALTFN_5,
  GPIO_ALTFN_6,
  GPIO_ALTFN_7,
  GPIO_ALTFN_ANALOG,
  NUM_GPIO_ALTFN,
} GPIOAltFn;

// GPIO settings for setting the value of a pin
typedef struct GPIOSettings {
  GPIODir direction;
  GPIOState state;
  GPIORes resistor;
  GPIOAltFn alt_function;
} GPIOSettings;

// All the following functions return true if the operation was successful and false otherwise.

// Initializes GPIO globally by setting all pins to their default state. ONLY CALL ONCE or it will
// deinit all current settings. Change setting by calling gpio_init_pin.
StatusCode gpio_init();

// Initializes a GPIO pin by address.
StatusCode gpio_init_pin(GPIOAddress *address, GPIOSettings *settings);

// Set the pin state by address.
StatusCode gpio_set_pin_state(GPIOAddress *address, GPIOState state);

// Toggles the output state of the pin.
StatusCode gpio_toggle_state(GPIOAddress *address);

// Gets the value of the input register for a pin and assigns it to the state that is passed in.
StatusCode gpio_get_value(GPIOAddress *address, GPIOState *input_state);
