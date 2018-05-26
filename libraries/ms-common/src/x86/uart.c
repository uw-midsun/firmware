#include "uart.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "x86_socket.h"

typedef struct {
  UARTPort port;
  X86SocketThread thread;
  UARTStorage *storage;
} UartSocketData;

static UartSocketData s_sock[NUM_UART_PORTS];

void uart_receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                           size_t rx_len, void *context);

void uart_receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                           size_t rx_len, void *context) {
  UartSocketData *sock = context;
  sock->storage->rx_handler((uint8_t *)rx_data, rx_len, context);
}

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {
  // create module name from UART_PORT number
  size_t module_name_len = 6;
  char *module_name = malloc(module_name_len);
  snprintf(module_name, module_name_len, "%s%d", "uart", ((int)uart + 1));

  s_sock[uart].port = uart;
  s_sock[uart].storage = storage;
  s_sock[uart].storage->rx_handler = settings->rx_handler;
  s_sock[uart].storage->context = settings->context;

  status_ok_or_return(x86_socket_init(&s_sock[uart].thread, module_name, uart_receiver_wrapper,
                                      (void *)&s_sock[uart]));

  return STATUS_CODE_OK;
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {
  s_sock[uart].storage->rx_handler = rx_handler;
  s_sock[uart].storage->context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(x86_socket_broadcast(&s_sock[uart].thread, (char *)tx_data, len));

  return STATUS_CODE_OK;
}
