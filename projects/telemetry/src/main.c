#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_hw.h"
#include "can_uart.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gps.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "uart.h"
#include "wait.h"
#include "xbee.h"

static CanUart s_can_uart_settings = {
  .uart = XBEE_UART_PORT,
  .rx_cb = NULL,
  .context = NULL,
};

static CANHwSettings can_hw_settings = {
  .tx = { .port = GPIO_PORT_A, .pin = 12 },
  .rx = { .port = GPIO_PORT_A, .pin = 11 },
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .loopback = false,
};

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

  ret = can_hw_init(&can_hw_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize can_hw. Quitting...\n");
  }

  ret = can_uart_init(&s_can_uart_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize can_uart. Quitting...\n");
    return 0;
  }

  ret = can_uart_enable_passthrough(&s_can_uart_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not enable uart passthrough. Quitting...\n");
  }

  while (true) {
    wait();
  }

  return 0;
}
