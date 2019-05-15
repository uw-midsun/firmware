#pragma once
// Push button module for LED control written for the Tutorial board
// This module is used as an introduction to Midnight Sun style firmware.
// It is organized in such a way as to expose the user to basic fundamentals
// and principals we follow when writing drivers and modules
// Requires interrupts to be initialized
#include "gpio.h"
#include "gpio_it.h"

// Type of Button
typedef enum {
  BUTTON_COLOUR_YELLOW = 0,
  BUTTON_COLOUR_GREEN,
  NUM_BUTTON_COLOURS,
} ButtonColour;

// Button settings structure
typedef struct ButtonSettings {
  GpioAddress button_addresses[NUM_BUTTON_COLOURS]; // Array of GPIO addresses for each push button
  GpioAddress led_addresses[NUM_BUTTON_COLOURS];  // Array of GPIO addresses for each LED
  GpioSettings gpio_settings; // GPIO settings for the push-button (input)
  GpioSettings led_settings; // GPIO settings for LED (output)
  InterruptSettings interrupt_settings; // Interrupt settings for push-buttons
  InterruptEdge interrupt_edge; // Interrupt trigger edge for push-buttons
} ButtonSettings;

// Button storage structure
typedef struct ButtonStorage {
  GpioAddress led_address; // GPIO address for the associated LED
  volatile uint32_t count; // Count of button presses
} ButtonStorage;

// Initializes the button GPIOs and corresponding LED outputs
StatusCode button_init(const ButtonSettings *settings, ButtonStorage *storage);
