#pragma once

#include <stdint.h>

#include "interrupt.h"
#include "status.h"

// Initializes interrupt internals. If called multiple times the subsequent attempts will not change
// anything.
void stm32f0xx_interrupt_init(void);

// Enables the nested interrupt vector controller for a given channel.
StatusCode stm32f0xx_interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority);

// Enables the external interrupt line with the given settings.
StatusCode stm32f0xx_interrupt_exti_enable(uint8_t line, InterruptSettings *settings, InterruptEdge edge);

// Triggers a software interrupt on a given external interrupt.
StatusCode stm32f0xx_interrupt_exti_trigger(uint8_t line);

// Get the pending flag for an external interrupt.
StatusCode stm32f0xx_interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit);

// Clears the pending flag for an external interrupt.
StatusCode stm32f0xx_interrupt_exti_clear_pending(uint8_t line);
