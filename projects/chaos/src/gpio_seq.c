#include "gpio_seq.h"

#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "status.h"

StatusCode gpio_seq_init_pins(const GpioAddress *addrs, size_t num_addrs,
                              const GPIOSettings *settings, uint32_t delay_time_us) {
  for (size_t i = 0; i < num_addrs; i++) {
    status_ok_or_return(gpio_init_pin(&addrs[i], settings));
    if (delay_time_us) {
      delay_us(delay_time_us);
    }
  }
  return STATUS_CODE_OK;
}

StatusCode gpio_seq_set_state(const GpioAddress *addrs, size_t num_addrs, GpioState state,
                              uint32_t delay_time_us) {
  for (size_t i = 0; i < num_addrs; i++) {
    status_ok_or_return(gpio_set_state(&addrs[i], state));
    if (delay_time_us) {
      delay_us(delay_time_us);
    }
  }
  return STATUS_CODE_OK;
}
