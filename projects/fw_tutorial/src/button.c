#include "button.h"
#include <stddef.h>
#include <stdio.h>

StatusCode button_init(const ButtonSettings *settings, ButtonStorage *storage) {
  // Argument checking
  if (settings == NULL || storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Initialize all push-buttons (including interrupts) and LEDs
  // Enter your code below ...

  // Everything has been initialized properly
  return STATUS_CODE_OK;
}
