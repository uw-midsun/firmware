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

GpioSettings telemetry_settings_gpio_general = {
  .direction = GPIO_DIR_OUT,  // The pin needs to output.
  .state = GPIO_STATE_LOW,    // Start in the "off" state.
  .alt_function = GPIO_ALTFN_1,
};

UartSettings telemetry_gps_uart_settings = {
  .baudrate = 9600,
  .tx = { .port = GPIO_PORT_B, .pin = 10 },
  .rx = { .port = GPIO_PORT_B, .pin = 11 },
  .alt_fn = GPIO_ALTFN_4,     //ALTFN for UART for PB10 and PB11
};

// The pin numbers to use for providing power and turning the GPS on and off
GpioAddress telemetry_gps_on_off_pin = { .port = GPIO_PORT_B, .pin = 9 };  // Pin GPS on_off

GpsSettings telemetry_gps_settings = { .pin_on_off = &telemetry_gps_on_off_pin,
                                       .uart_settings = &telemetry_gps_uart_settings,
                                       .port = UART_PORT_3 };

GpsStorage telemetry_gps_storage = { 0 };

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();

  // Initialize UART
  StatusCode ret = uart_init( telemetry_gps_settings->port, 
                              telemetry_gps_settings->uart_settings, 
                              &telemetry_gps_settings->uart_storage);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Error initializing UART\n");
  }

  ret |= gps_init(&telemetry_gps_settings, &telemetry_gps_storage);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize GPS\n");
  }

  while (true) {
  }

  return 0;
}
