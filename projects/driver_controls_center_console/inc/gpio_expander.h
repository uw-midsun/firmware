#pragma once

// Driver for the MCP23008 GPIO Expander
// Requires GPIO, Interrupts, Soft Timers, and I2C to be initialized
//
// Periodically polls for updates in case we missed an interrupt somehow

#include "gpio.h"
#include "i2c.h"

#define GPIO_EXPANDER_POLL_PERIOD_MS 100

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
} GpioExpanderPin;

typedef enum {
  GPIO_EXPANDER_ADDRESS_0 = 0,
  GPIO_EXPANDER_ADDRESS_1,
  GPIO_EXPANDER_ADDRESS_2,
  GPIO_EXPANDER_ADDRESS_3,
  GPIO_EXPANDER_ADDRESS_4,
  GPIO_EXPANDER_ADDRESS_5,
  GPIO_EXPANDER_ADDRESS_6,
  GPIO_EXPANDER_ADDRESS_7,
  NUM_GPIO_EXPANDER_ADDRESSES
} GpioExpanderAddress;

typedef void (*GpioExpanderCallbackFn)(GpioExpanderPin pin, GpioState state, void *context);

typedef struct GpioExpanderCallback {
  GpioExpanderCallbackFn func;
  void *context;
} GpioExpanderCallback;

typedef struct GpioExpanderStorage {
  I2CPort port;
  I2CAddress addr;
  GpioExpanderCallback callbacks[NUM_GPIO_EXPANDER_PINS];
  uint8_t prev_state;
} GpioExpanderStorage;

// Initialize the expander with an address to connect to its INT pin.
// If |interrupt_pin| is NULL, it assumes the IO expander is output-only.
StatusCode gpio_expander_init(GpioExpanderStorage *expander, I2CPort port, GpioExpanderAddress addr,
                              const GpioAddress *interrupt_pin);

// Initialize one of the expander pins.
StatusCode gpio_expander_init_pin(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                  const GpioSettings *settings);

// Returns the set output value or the current input value based on the pin configuration
StatusCode gpio_expander_get_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GpioState *state);

// Set the state of an output pin
StatusCode gpio_expander_set_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GpioState state);

// Register a callback for a specific pin
StatusCode gpio_expander_register_callback(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                           GpioExpanderCallbackFn callback, void *context);
