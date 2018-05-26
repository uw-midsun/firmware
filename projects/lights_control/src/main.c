#include <stdint.h>

#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "lights_gpio.h"

int main(void) {
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  while (true) {
  }
  return STATUS_CODE_OK;
}
