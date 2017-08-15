#pragma once
// GPIO Interrupt Handlers
// Requires GPIO and interrupts to be initialized.
#include <stdint.h>

#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"

typedef void (*gpio_it_callback)(GPIOAddress *address, void *context);

// Initializes the interrupt handler for GPIO.
void gpio_it_init(void);

// Registers a new callback on a given port pin combination with the desired settings.
StatusCode gpio_it_register_interrupt(GPIOAddress *address, InterruptSettings *settings,
                                      InterruptEdge edge, gpio_it_callback callback, void *context);

// Triggers an interrupt in software.
StatusCode gpio_it_trigger_interrupt(GPIOAddress *address);
