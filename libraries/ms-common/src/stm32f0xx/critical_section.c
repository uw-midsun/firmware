#include "critical_section.h"

#include <stdbool.h>

#include "stm32f0xx_misc.h"

CriticalSection critical_section_start(void) {
  CriticalSection section = {
    .disabled_in_scope = false,
    .requires_cleanup = true,
  };

  if (!__get_PRIMASK()) {
    __disable_irq();
    // Interrupts got disabled.
    section.disabled_in_scope = true;
  }

  return section;
}

void critical_section_end(CriticalSection *section) {
  if (section->requires_cleanup) {
    section->requires_cleanup = false;
    if (__get_PRIMASK() && section->disabled_in_scope) {
      __enable_irq();
    }
  }
}
