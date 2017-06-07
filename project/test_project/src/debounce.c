#include "debounce.h"

void debounce(GPIOAddress* address, GPIOState* current_state) {
  uint16_t count = HOLD_TIME;
  GPIOState prev_state = 0;

  while (count > 0) {
    for (uint8_t i = 0; i < SAMPLING_INTERVAL; i++) {
      gpio_get_value(address, current_state);
    }

    if (*current_state == prev_state) {
      count--;
    } else {
      count = HOLD_TIME;
    }
  prev_state = *current_state;
  }
}
