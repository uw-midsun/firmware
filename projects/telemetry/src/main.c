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
#include "nmea.h"
#include "soft_timer.h"
#include "uart.h"
#include "wait.h"
#include "xbee.h"

GpioSettings telemetry_settings_gpio_general = {
  .direction = GPIO_DIR_OUT,  // The pin needs to output.
  .state = GPIO_STATE_LOW,    // Start in the "off" state.
  .alt_function = GPIO_ALTFN_1,
};

UartSettings telemetry_gps_uart_settings = {
  .baudrate = 9600,
  .tx = { .port = GPIO_PORT_A, .pin = 2 },
  .rx = { .port = GPIO_PORT_A, .pin = 3 },
  .alt_fn = GPIO_ALTFN_1,
};

// The pin numbers to use for providing power and turning the GPS on and off
GpioAddress telemetry_gps_pins[] = {
  { .port = GPIO_PORT_B, .pin = 3 },  // Pin GPS power
  { .port = GPIO_PORT_B, .pin = 4 },  // Pin GPS on_off
};

GpsSettings telemetry_gps_settings = { .pin_power = &telemetry_gps_pins[0],
                                       .pin_on_off = &telemetry_gps_pins[1],
                                       .uart_settings = &telemetry_gps_uart_settings,
                                       .port = UART_PORT_2 };

GpsStorage telemetry_gps_storage = { 0 };

CanUart telemetry_can_uart_settings = {
  .uart = XBEE_UART_PORT,
  .rx_cb = NULL,
  .context = NULL,
};

CanHwSettings telemetry_can_hw_settings = {
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

  ret = gps_init(&telemetry_gps_settings, &telemetry_gps_storage);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize GPS\n");
  }

  ret = can_hw_init(&telemetry_can_hw_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize can_hw. Quitting...\n");
  }

  ret = can_uart_init(&telemetry_can_uart_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize can_uart. Quitting...\n");
    return 0;
  }

  ret = can_uart_enable_passthrough(&telemetry_can_uart_settings);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not enable uart passthrough. Quitting...\n");
  }

  while (true) {
    wait();
  }

  return 0;
}
