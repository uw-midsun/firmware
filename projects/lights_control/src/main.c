#include <stdint.h>

#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "lights_can.h"
#include "lights_gpio.h"
#include "simple_peripherals.h"

#define SOMEPORT 0
#define SOMEPIN 0

int main(void) {
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  BoardType boardtype;
  get_board_type(&boardtype);

  lights_can_init(boardtype);
  lights_gpio_init(boardtype);
  simple_peripherals_init();

  while (true) {
    StatusCode status = event_process(&e);
    if (status == STATUS_CODE_OK) {
    } else if (status == STATUS_CODE_EMPTY) {
      wait();
    }
  }

  return STATUS_CODE_OK;
}
