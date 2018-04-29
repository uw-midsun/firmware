#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "gpio_it.h"

void callback(const GPIOAddress *address, void *context) {
  printf("callback\n");
}

int main(void) {
  gpio_init();

  GPIOAddress address = { GPIO_PORT_A, 0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING_FALLING, callback, NULL);

  while (1) {
  }

  return 0;
}
