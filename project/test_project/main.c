#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"

int main(void) {
  GPIOSettings settings = {GPIO_DIR_OUT, GPIO_STATE_HIGH, GPIO_RES_NONE, GPIO_ALTFN_NONE};
  GPIOAddress addr[] = {{2, 6}, {2, 7}, {2, 8}, {2, 9}};
  const uint32_t addr_size = sizeof(addr) / sizeof(GPIOAddress);

  gpio_init();

  volatile uint32_t i;
  for (i = 0; i < addr_size; i++) {
    gpio_init_pin(&addr[i], &settings);
  }

  volatile uint32_t interval = 0;
  volatile uint32_t j;

  while (true) {
    for (i = 0; i < interval; i++) {
    }
    for (j = 0; j < addr_size; j++) {
      gpio_toggle_state(&addr[j]);
    }

    interval += 10;
  }
}
