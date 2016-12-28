#pragma once
// Shared interface elements across all devices for interrupts. Each device will also have internal
// device specific functionality which exist in their respective interrupt_mcu.h files (do not
// include that file directly instead reference it through a module such as gpio_it.h).

#include <stdbool.h>

// A generic interrupt callback with context.
typedef void (*interrupt_callback)(void *context);

// The interrupt type runs a callback as soon as the interrupt is triggered. The event type raises a
// flag which needs to be checked/polled periodically and then the callback is run.
typedef enum {
  INTERRUPT_TYPE_INTERRUPT = 0,
  INTERRUPT_TYPE_EVENT,
  NUM_INTERRUPT_TYPE,
} InterruptType;

typedef enum {
  INTERRUPT_PRIORITY_HIGH = 0,
  INTERRUPT_PRIORITY_NORMAL,
  INTERRUPT_PRIORITY_LOW,
  NUM_INTERRUPT_PRIORITY,
} InterruptPriority;

// Defines on what edge of an input signal the interrupt triggers on. This is not necessarily
// applicable for all interrupts; however, external interrupts will use them.
typedef enum {
  INTERRUPT_EDGE_RISING = 0,
  INTERRUPT_EDGE_FALLING,
  INTERRUPT_EDGE_RISING_FALLING,
  NUM_INTERRUPT_EDGE,
} InterruptEdge;

typedef struct InterruptSettings {
  InterruptType type;
  InterruptPriority priority;
} InterruptSettings;

// To protect critical code use the following two functions:
// EXAMPLE:
//
// bool was_disabled = interrupt_disabled();
// // Critical code here.
// interrupt_enable(was_disabled);
// ...
//
// This will also protect nested attempts from enabling and disabling interupts from prematurely
// ending the critical section.

// Enables all registered interrupts on all line/inputs.
void interrupt_enable(bool disabled_in_scope);

// Disables all interrupts accross all lines/inputs. Returns true if interrupts were previously
// disabled.
bool interrupt_disable();
