#include <stdio.h>
#include <string.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"
#include "nmea.h"

static const UARTPort port = UART_PORT_2;
static GPSHandler gps_handler;

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  // Check that the context is correct

  NMEAResult r = parse_nmea_sentence(rx_arr, len);
  gps_handler(r);
}

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(UARTSettings* uart_settings, GPSHandler handler) {
  gps_handler = handler;
  uart_settings->rx_handler = s_nmea_read;
  // Makes sure that status codes are handled
  status_ok_or_return(uart_init(port, uart_settings, &s_storage));
  
  uint8_t data[2] = { 42, 24 };

  status_ok_or_return(uart_tx(port, data, SIZEOF_ARRAY(data)));
  return STATUS_CODE_OK;
}
