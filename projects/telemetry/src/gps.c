#include <stdio.h>
#include <string.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"
#include "nmea.h"

static const UARTPort port = UART_PORT_2;

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  // Check that the context is correct
  
  NMEAResult r = parse_nmea_sentence(rx_arr, len);
}

// FIXME: move this to main.c and pass it in as a arg to evm_gps_init
static UARTSettings s_settings = {
  .baudrate = 9600,
  .rx_handler = s_nmea_read,

  .tx = { .port = GPIO_PORT_A , .pin = 2 },
  .rx = { .port = GPIO_PORT_A , .pin = 3 },
  .alt_fn = GPIO_ALTFN_1,
};

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(void) {
  // Makes sure that status codes are handled
  
  status_ok_or_return(uart_init(port, &s_settings, &s_storage));
  
  uint8_t data[2] = { 42, 24 };
  
  status_ok_or_return(uart_tx(port, data, SIZEOF_ARRAY(data)));
  return STATUS_CODE_OK;
}
