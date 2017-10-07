#include <stdio.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"

static const UARTPort port = UART_PORT_2;

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  printf("recieved data with len %d\n", len);
  for (uint8_t i = 0; i < len; i++) {
    printf("byte %d: %d\n", i, rx_arr[i]);
  }
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
  uart_init(port, &s_settings, &s_storage);
  uint8_t data[2] = { 42, 24 };
  uart_tx(port, data, SIZEOF_ARRAY(data));
  return STATUS_CODE_OK;
}
