#include "gpio_seq.h"

#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "status.h"

static uint32_t s_delay_us;

void gpio_seq_set_delay(uint32_t delay_us) {
  s_delay_us = delay_us;
}

uint32_t gpio_seq_get_delay(void) {
  return s_delay_us;
}

StatusCode gpio_seq_init_pins(const GPIOAddress *addrs, size_t n_addrs,
                              const GPIOSettings *settings) {
  for (size_t i = 0; i < n_addrs; i++) {
    status_ok_or_return(gpio_init_pin(&addrs[i], settings));
    if (s_delay_us) {
      delay_us(s_delay_us);
    }
  }
  return STATUS_CODE_OK;
}

StatusCode gpio_seq_set_state(const GPIOAddress *addrs, size_t n_addrs, GPIOState state) {
  for (size_t i = 0; i < n_addrs; i++) {
    status_ok_or_return(gpio_set_state(&addrs[i], state));
    if (s_delay_us) {
      delay_us(s_delay_us);
    }
  }
  return STATUS_CODE_OK;
}
