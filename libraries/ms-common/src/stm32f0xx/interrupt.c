#include "interrupt.h"

#include <stdbool.h>

#include "stm32f0xx_misc.h"

static bool s_interrupts_disabled = false;

void interrupt_enable(bool disabled_in_scope) {
  if (s_interrupts_disabled && disabled_in_scope) {
    __enable_irq();
    s_interrupts_disabled = false;
  }
}

bool interrupt_disable() {
  if (!s_interrupts_disabled) {
    __disable_irq();
    // Interrupts were previously not disabled.
    return false;
  }
  // Interrupts were previously disabled.
  return true;
}
