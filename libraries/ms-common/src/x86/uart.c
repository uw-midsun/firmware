#include "uart.h"

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UartStorage *storage) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_set_delimiter(UARTPort uart, uint8_t delimiter) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
