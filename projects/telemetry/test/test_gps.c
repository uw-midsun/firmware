#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gps.h"
#include "interrupt.h"
#include "log.h"
#include "nmea.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "uart.h"
#include "unity.h"

#define TELEMETRY_GPS_UART_BAUD_RATE 9600;
#define TELEMETRY_GPS_UART_PORT UART_PORT_3;
#define TELEMETRY_GPS_UART_TX \
  { .port = GPIO_PORT_B, .pin = 10 }
#define TELMETRY_GPS_UART_RX \
  { .port = GPIO_PORT_B, .pin = 11 }
#define TELEMETRY_GPS_UART_ALTFN GPIO_ALTFN_4

static UartStorage s_uart_storage;

UartSettings telemetry_gps_uart_settings = {
  .baudrate = TELEMETRY_GPS_UART_BAUD_RATE,
  .tx = TELMETRY_GPS_UART_TX,
  .rx = TELMETRY_GPS_UART_RX,
  .alt_fn = TELEMETRY_GPS_UART_ALTFN  // ALTFN for UART for PB10 and PB11
};

// The pin numbers to use for providing power and turning the GPS on and off
GpioAddress telemetry_gps_on_off_pin = { .port = GPIO_PORT_B, .pin = 9 };  // Pin GPS power

GpsSettings telemetry_gps_settings = { .pin_on_off = &telemetry_gps_on_off_pin,
                                       .port = UART_PORT_3 };

GpsStorage telemetry_gps_storage = { 0 };

void setup_test(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();

  uart_init(TELEMETRY_GPS_UART_PORT, &telemetry_gps_uart_settings, &telemetry_gps_uart_storage);
}

void teardown_test(void) {}

void test_gps_guards(void) {
  uint8_t *gga_test = NULL;
  uint8_t *vtg_test = NULL;
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, gps_get_gga_data(&gga_test));
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, gps_get_vtg_data(&vtg_test));
}

void test_gps_output(void) {
  uint8_t *gga_result = NULL;
  uint8_t *vtg_result = NULL;

  TEST_ASSERT_OK(gps_init(&telemetry_gps_settings, &telemetry_gps_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED,
                    gps_init(&telemetry_gps_settings, &telemetry_gps_storage));

  delay_s(2);

  TEST_ASSERT_OK(gps_get_gga_data(&gga_result));
  TEST_ASSERT_OK(gps_get_vtg_data(&vtg_result));
}
