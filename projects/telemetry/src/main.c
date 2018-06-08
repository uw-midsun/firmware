#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gps.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "uart.h"
#include "xbee.h"

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();

  StatusCode ret = xbee_init();
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize xbee. Quitting...\n");
    return 0;
  }

  ret = gps_init();
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize GPS\n");
  }

  // Integrate GPS with xbee here
  return 0;
}
