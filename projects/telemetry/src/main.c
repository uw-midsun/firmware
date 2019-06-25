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

#include "gpio_it.h"
#include "can.h"

#include "can_msg_defs.h"

#include "gps_cfg.h"
typedef enum {
  GPS_EVENT_SYSTEM_CAN_RX = 0,
  GPS_EVENT_SYSTEM_CAN_TX,
  GPS_EVENT_SYSTEM_CAN_FAULT,
} GpsEvent;

static CanStorage s_can_storage = { 0 };

static UartStorage s_uart_storage;

UartSettings telemetry_gps_uart_settings = {
  .baudrate = TELEMETRY_GPS_UART_BAUD_RATE,
  .tx = TELEMETRY_GPS_UART_TX,
  .rx = TELEMETRY_GPS_UART_RX,
  .alt_fn = TELEMETRY_GPS_UART_ALTFN,  // ALTFN for UART for PB10 and PB11
  .context = NULL,
};

GpioAddress telemetry_on_off = TELEMETRY_GPS_ON_OFF_PIN; 

GpsSettings telemetry_gps_settings = {
  .pin_on_off = &telemetry_on_off,
  .uart_port = UART_PORT_3,
};

GpsStorage telemetry_gps_storage = { 0 };

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  Event e = { 0 };

  // Initialize GPIO
  GpioSettings telemetry_settings_gpio_general = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };
  status_ok_or_return(gpio_init_pin(telemetry_gps_settings.pin_on_off, &telemetry_settings_gpio_general));

  // Initialize CAN
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_TELEMETRY,
    .bitrate = GPS_CAN_BITRATE,
    .rx_event = GPS_EVENT_SYSTEM_CAN_RX,
    .tx_event = GPS_EVENT_SYSTEM_CAN_TX,
    .fault_event = GPS_EVENT_SYSTEM_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Initialize UART
  status_ok_or_return(uart_init(TELEMETRY_GPS_UART_PORT, &telemetry_gps_uart_settings, &s_uart_storage));

  // Initialize GPS
  status_ok_or_return(gps_init(&telemetry_gps_settings, &telemetry_gps_storage));

  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }
  return 0;
}
