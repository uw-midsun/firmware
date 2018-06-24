#include <stdbool.h>

#include "debouncer.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_callback(const GPIOAddress *address, void *context) {
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

  GPIOAddress button = {
    .port = GPIO_PORT_A,  //
    .pin = 0,             //
  };

  DebouncerInfo debouncer_info = { 0 };

  debouncer_init_pin(&debouncer_info, &button, prv_callback, NULL);

  // Add infinite loop so we don't exit
  while (true) {
    wait();
  }

  return 0;
}