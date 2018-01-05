#pragma once
// GPIO event sequencing with delays between actions.
// Requires initialization of gpio, soft_timer and interrupts.

#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

// Sets the delay time between updating pins, defaults to no delay.
void gpio_seq_set_delay(uint32_t delay_us);

// Gets the delay time between updating pins.
uint32_t gpio_seq_get_delay(void);

// Initializes the list of pins provided.
StatusCode gpio_seq_init_pins(const GPIOAddress *addrs, size_t n_addrs,
                              const GPIOSettings *settings);

// Sets the state of the pins provided. Must be initialized first or unexpected behavior may ensue.
StatusCode gpio_seq_set_state(const GPIOAddress *addrs, size_t n_addrs, GPIOState state);
