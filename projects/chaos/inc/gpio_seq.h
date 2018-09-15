#pragma once
// GPIO event sequencing with delays between actions.
// Requires initialization of gpio, soft_timer and interrupts.

#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

// Initializes the list of pins provided.
StatusCode gpio_seq_init_pins(const GpioAddress *addrs, size_t num_addrs,
                              const GPIOSettings *settings, uint32_t delay_time_us);

// Sets the state of the pins provided. Must be initialized first or unexpected behavior may ensue.
StatusCode gpio_seq_set_state(const GpioAddress *addrs, size_t num_addrs, GPIOState state,
                              uint32_t delay_time_us);
