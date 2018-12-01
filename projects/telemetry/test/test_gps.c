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

void setup_test(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_gps_guards(void) {
  char *gga_test;
  char *vtg_test;
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, gps_get_gga_data(&gga_test));
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, gps_get_vtg_data(&vtg_test));
}

void test_gps_output(void) {
  char *gga_result;
  char *vtg_result;

  TEST_ASSERT_OK(gps_init(&telemetry_gps_settings, &telemetry_gps_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED,
                    gps_init(&telemetry_gps_settings, &telemetry_gps_storage));

  delay_s(2);

  TEST_ASSERT_OK(gps_get_gga_data(&gga_result));
  TEST_ASSERT_OK(gps_get_vtg_data(&vtg_result));
}
