#include <stdbool.h>
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

#include "delay.h"
#include "interrupt_def.h"

#include "stm32f0xx.h"
#include "wait.h"

#include "debouncer.h"
#include "soft_timer.h"

void callback(const GPIOAddress *address, void *context) {
  LOG_DEBUG("switched\n");
}

static void prv_handle_statuscode(const Status *status) {
  LOG_DEBUG("Status code: %d from %s (caller %s): %s\n", status->code, status->source,
            status->caller, status->message);
}

int main(void) {
  LOG_DEBUG("Hello World!\n");

  // Init GPIO module
  gpio_init();
  interrupt_init();
  soft_timer_init();
  status_register_callback(prv_handle_statuscode);
  // Set up PA0 as an output, default to output 0 (GND)
  GPIOAddress button = {
    .port = GPIO_PORT_A,  //
    .pin = 0,             //
  };

  DebounceInfo db = { 0 };

  debouncer_init_pin(&db, &button, callback, NULL);

  // Add infinite loop so we don't exit
  while (true) {
    wait();
  }

  return 0;
}
