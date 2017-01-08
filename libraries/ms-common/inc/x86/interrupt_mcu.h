#pragma once

#include <stdint.h>

#include "interrupt.h"
#include "status.h"

typedef void (*interrupt_mcu_handler)(uint8_t interrupt_id);

// Initializes the interrupt internals. If called multiple times the subsequent attempts will not
// change anything.
void interrupt_mcu_init(void);

// Registers an ISR handler. The handler_id is updated to the id assigned to the handler if
// registered successfully.
StatusCode interrupt_mcu_register_handler(interrupt_mcu_handler handler, uint8_t* handler_id);

// Registers a callback to a handler assigning it a new id from the global interrupt id pool.
StatusCode interrupt_mcu_register_interrupt(uint8_t handler_id, InterruptSettings* settings,
                                            uint8_t* interrupt_id);

// Triggers a software interrupt by interrupt_id.
StatusCode interrupt_mcu_trigger(uint8_t interrupt_id);
