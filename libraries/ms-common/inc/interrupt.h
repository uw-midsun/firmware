#pragma once

#include "gpio.h"
#include "status.h"

typedef void (*interrupt_callback)(void);

typedef enum {
  INTERRUPT_TYPE_INTERRUPT = 0,
  INTERRUPT_TYPE_EVENT,
  NUM_INTERRUPT_TYPE,
} InterruptType;

typedef enum {
  INTERRUPT_EDGE_RISING = 0,
  INTERRUPT_EDGE_RISING_FALLING,
  INTERRUPT_EDGE_FALLING,
  NUM_INTERRUPT_EDGE,
} InterruptEdge;

typedef enum {
  INTERRUPT_PRIORITY_HIGH = 0,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_PRIORITY_LOW,
  NUM_INTERRUPT_PRIORITY,
} InterruptPriority;

typedef struct InterruptSettings {
  InterruptType type;
  InterruptEdge edge;
  InterruptPriority priority;
} InterruptSettings;

// Initialize interrupts.
void interrupt_init();

// Enables all registered interrupts on all line/inputs.
void interrupt_enable();

// Disables all interrupts accross all lines/inputs.
void interrupt_disable();

// CRITICAL SECTIONS
// Annotates a critical section.
// TODO(ELEC-43) Enforce that this pair must be called in order within the same scope in the
// preprocessor/compiler. This is to prevent the accidental permanent disabling of interrupts after
// a critical section.

#define BEGIN_CRITICAL_SECTION() interrupt_disable()

#define END_CRITICAL_SECTION() interrupt_enable()

// GPIO INTERRUPTS

// Register a gpio interrupt for the provided address, settings and using the provided callback.
// WARNING: ON STM32F0XX ONLY ONE INTERRUPT CAN BE REGISTERED FOR A GIVEN PIN NUMBER!!
// ie: registering port 1 pin 0 will fail if you already registered port 0 pin 1.
StatusCode interrupt_gpio_register(GPIOAddress *address, InterruptSettings *settings,
                                   interrupt_callback callback);

// Trigger a software interrupt for the GPIOAddress provided.
StatusCode interrupt_gpio_trigger(GPIOAddress *address);
