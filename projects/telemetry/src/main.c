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

#define TELEMETRY_GPS_UART_BAUD_RATE 9600
#define TELEMETRY_GPS_UART_PORT UART_PORT_3
#define TELEMETRY_GPS_UART_TX \
  { .port = GPIO_PORT_B, .pin = 10 }
#define TELEMETRY_GPS_UART_RX \
  { .port = GPIO_PORT_B, .pin = 11 }
#define TELEMETRY_GPS_UART_ALTFN GPIO_ALTFN_4

static UartStorage s_uart_storage;

UartSettings telemetry_gps_uart_settings = {
  .baudrate = TELEMETRY_GPS_UART_BAUD_RATE,
  .tx = TELEMETRY_GPS_UART_TX,
  .rx = TELEMETRY_GPS_UART_RX,
  .alt_fn = TELEMETRY_GPS_UART_ALTFN  // ALTFN for UART for PB10 and PB11
};

// The pin numbers to use for providing power and turning the GPS on and off
GpioAddress telemetry_gps_on_off_pin = { .port = GPIO_PORT_B, .pin = 9 };  // Pin GPS on_off

GpsSettings telemetry_gps_settings = { .pin_on_off = &telemetry_gps_on_off_pin,
                                       .uart_port = UART_PORT_3 };

GpsStorage telemetry_gps_storage = { 0 };

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  
  Event e = { 0 };

  const CanSettings can_settings = {
    .device_id = DC_CFG_CAN_DEVICE_ID,
    .bitrate = DC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = DC_CFG_CAN_TX,
    .rx = DC_CFG_CAN_RX,
    .loopback = false,
  };

  StatusCode status = can_init(&s_can_storage, &can_settings);

  // Initialize UART
  StatusCode ret =
      uart_init(TELEMETRY_GPS_UART_PORT, &telemetry_gps_uart_settings, &s_uart_storage);

  if (!status_ok(ret)) {
    LOG_CRITICAL("Error initializing UART\n");
  }

  ret |= gps_init(&telemetry_gps_settings, &telemetry_gps_storage);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Telemetry project could not initialize GPS\n");
  }

  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}
