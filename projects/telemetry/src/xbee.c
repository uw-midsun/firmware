#include "xbee.h"

// Sends data over UART to the xbee, RX info is for completeness sake
#define TELEMETRY_XBEE_PIN_ALT_FN GPIO_ALTFN_4

#define TELEMETRY_XBEE_PORT_TX GPIO_PORT_B
#define TELEMETRY_XBEE_PIN_TX 10

#define TELEMETRY_XBEE_PORT_RX GPIO_PORT_B
#define TELEMETRY_XBEE_PIN_RX 11

static const UARTPort s_xbee_port = UART_PORT_3;

static UARTSettings s_xbee_settings = {
  .baudrate = 9600,  // Baurate to be changed
  .tx = { .port = TELEMETRY_XBEE_PORT_TX, .pin = TELEMETRY_XBEE_PIN_TX },
  .rx = { .port = TELEMETRY_XBEE_PORT_RX, .pin = TELEMETRY_XBEE_PIN_RX },
  .alt_fn = TELEMETRY_XBEE_PIN_ALT_FN,
};

static UARTStorage s_storage;

StatusCode xbee_init() {
  status_ok_or_return(uart_init(s_xbee_port, &s_xbee_settings, &s_storage));
  return STATUS_CODE_OK;
}

StatusCode xbee_transmit(const uint8_t *message, size_t len) {
  if (len == 0 || message[len - 1] != '\n') {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Must be a NULL terminated message");
  }
  status_ok_or_return(uart_tx(s_xbee_port, message, len));
  return STATUS_CODE_OK;
}
