#pragma once

#include <stdint.h>

#include "interrupt.h"
#include "status.h"

typedef void (*x86_interrupt_handler)(uint8_t interrupt_id);

// Initializes the interrupt internals. If called multiple times the subsequent attempts will clear
// everything resulting in the need to re initialize all interrupts.
void x86_interrupt_init(void);

// Registers an ISR handler. The handler_id is updated to the id assigned to the handler if
// registered successfully.
StatusCode x86_interrupt_register_handler(x86_interrupt_handler handler, uint8_t* handler_id);

// Registers a callback to a handler assigning it a new id from the global interrupt id pool.
StatusCode x86_interrupt_register_interrupt(uint8_t handler_id, InterruptSettings* settings,
                                            uint8_t* interrupt_id);

// Triggers a software interrupt by interrupt_id.
StatusCode x86_interrupt_trigger(uint8_t interrupt_id);
