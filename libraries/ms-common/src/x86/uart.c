#include "uart.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "x86_socket.h"

typedef struct {
  UARTPort port;
  X86SocketThread thread;
  UARTStorage *storage;
  UARTSettings *settings;
} SocketData;

static SocketData s_sock[NUM_UART_PORTS];

static void receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                             size_t rx_len, void *context);

void receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data,
                      size_t rx_len, void *context) {
  // get port number from module_name "uart"+UARTPort
  UARTPort uart = atoi(thread->module_name + 4) - 1;
  s_sock[uart].storage->rx_handler((uint8_t *)rx_data, rx_len, context);
}

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {
  // create module name from UART_PORT number
  char *module_name = malloc(6 * sizeof(char));
  snprintf(module_name, sizeof(module_name), "%s%d", "uart", ((int)uart + 1));

  s_sock[uart].port = uart;
  s_sock[uart].storage = storage;
  s_sock[uart].settings = settings;

  status_ok_or_return(x86_socket_init(&s_sock[uart].thread, module_name, receiver_wrapper,
                                      (void *)s_sock[uart].storage));

  return STATUS_CODE_OK;
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {
  s_sock[uart].storage->rx_handler = rx_handler;
  s_sock[uart].storage->context = context;
  s_sock[uart].thread.context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(x86_socket_broadcast(&s_sock[uart].thread, (char *)tx_data, len));

  return STATUS_CODE_OK;
}
