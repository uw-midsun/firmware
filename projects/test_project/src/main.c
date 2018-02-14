#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "x86_cmd.h"
#include "gpio.h"

void handler(const char *cmd_str, void *context) {
  printf("Test: received [%s]\n", cmd_str);
}

int main(void) {
  x86_cmd_register_handler("test", handler, NULL);
  gpio_init();

  while (1) { }

  return 0;
}
