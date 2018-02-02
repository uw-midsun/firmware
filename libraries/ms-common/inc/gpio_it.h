#pragma once
// GPIO Interrupt Handlers
// Requires GPIO and interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"

typedef void (*gpio_it_callback)(const GPIOAddress *address, void *context);

// Initializes the interrupt handler for GPIO.
void gpio_it_init(void);

// Registers a new callback on a given port pin combination with the desired settings.
StatusCode gpio_it_register_interrupt(const GPIOAddress *address, const InterruptSettings *settings,
                                      InterruptEdge edge, gpio_it_callback callback, void *context);

// Triggers an interrupt in software.
StatusCode gpio_it_trigger_interrupt(const GPIOAddress *address);

// Masks the interrupt for the given address if masked is True.
// Enables the interrupt if masked is false.
StatusCode gpio_it_mask_interrupt(const GPIOAddress *address, bool masked);
