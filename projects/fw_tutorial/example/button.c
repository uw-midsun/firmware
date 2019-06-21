#include "button.h"
#include <stddef.h>
#include <stdio.h>

static void prv_button_callback(const GpioAddress *address, void *context) {
  ButtonStorage *storage = (ButtonStorage *)context;

  // Increment counter
  storage->count++;

  // Toggle the appropriate LED
  gpio_toggle_state(&storage->led_address);
}

StatusCode button_init(const ButtonSettings *settings, ButtonStorage *storage) {
  // Argument checking
  if (settings == NULL || storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // GPIO settings for push-button
  GpioSettings gpio_settings = { .direction = GPIO_DIR_IN,
                                 .state = GPIO_STATE_LOW,
                                 .resistor = GPIO_RES_NONE,
                                 .alt_function = GPIO_ALTFN_NONE };

  // GPIO settings for LED
  GpioSettings led_settings = { .direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_NONE };

  // Initialize all the buttons
  for (uint8_t i = 0; i < NUM_BUTTON_COLOURS; i++) {
    // Store the LED address
    storage[i].led_address = settings->led_addresses[i];

    // Initialize the LED GPIO
    status_ok_or_return(gpio_init_pin(&settings->led_addresses[i], &led_settings));

    // Initialize the button GPIO and register its interrupt
    status_ok_or_return(gpio_init_pin(&settings->button_addresses[i], &gpio_settings));
    status_ok_or_return(
        gpio_it_register_interrupt(&settings->button_addresses[i], &settings->interrupt_settings,
                                   settings->interrupt_edge, prv_button_callback, &storage[i]));
  }

  // Everything has been initialized properly
  return STATUS_CODE_OK;
}
