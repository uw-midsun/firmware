#include <stdint.h>
#include <stdio.h>

#include "center_console.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "power_distribution_controller.h"
#include "soft_timer.h"

static GpioExpanderStorage s_expander;

int main() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const I2CSettings settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = { GPIO_PORT_B, 9 },  //
    .scl = { GPIO_PORT_B, 8 },  //
  };

  i2c_init(I2C_PORT_1, &settings);

  GPIOAddress int_pin = { GPIO_PORT_A, 9 };
  gpio_expander_init(&s_expander, I2C_PORT_1, GPIO_EXPANDER_ADDRESS_1, &int_pin);
  center_console_init(&s_expander);

  LOG_DEBUG("hello\n");

  Event e;
  for (;;) {
    if (status_ok(event_process(&e))) {
      // Process the event with the input FSMs
      power_distribution_controller_retry(&e);

      printf("Event %d\n", e.id);
    }
  }
}
