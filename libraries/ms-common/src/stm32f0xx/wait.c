#include "wait.h"

#include <stdbool.h>

#include "critical_section.h"
#include "stm32f0xx.h"

void wait(void) {
  bool disabled = critical_section_start();
  __WFI();
  critical_section_end(disabled);
}
