#include "xbee.h"

static const UARTPort s_xbee_port = XBEE_UART_PORT;

static UARTSettings s_xbee_settings = {
  .baudrate = 9600,  // Baurate to be changed
  .tx = { .port = GPIO_PORT_B, .pin = 10 },
  .rx = { .port = GPIO_PORT_B, .pin = 11 },
  .alt_fn = GPIO_ALTFN_4,
};

static UARTStorage s_storage;

StatusCode xbee_init() {
  return uart_init(s_xbee_port, &s_xbee_settings, &s_storage);
}

StatusCode xbee_transmit(const uint8_t *message, size_t len) {
  uint8_t newline = '\n';
  status_ok_or_return(uart_tx(s_xbee_port, message, len));
  return uart_tx(s_xbee_port, &newline, sizeof(newline));
}
