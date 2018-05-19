#include "uart.h"
#include "log.h"
#include "x86_socket.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
  UARTPort port;
  X86SocketThread thread;
  UARTStorage storage;
} SocketData;

static SocketData *sock;
static unsigned long num_sock = 0;

static int get_sock(UARTPort uart);
static void receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data, size_t rx_len, void *context);

int get_sock(UARTPort uart) {

  for(unsigned long current_sock = 0; current_sock < num_sock; current_sock++) {
    if(sock[current_sock].port == uart) {
      return (int)current_sock;
    }
  }

  return -1;
}

void receiver_wrapper(struct X86SocketThread *thread, int client_fd, const char *rx_data, size_t rx_len, void *context) {

  // get port number from module_name
  UARTPort port = atoi(thread->module_name) - 1;
  int current_sock = get_sock(port);

  sock[current_sock].storage.rx_handler((uint8_t*)rx_data, rx_len, context);
}

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {

  // append another SocketData struct to the array sock[]
  unsigned long latest_sock = num_sock;
  SocketData *tmp = sock;
  num_sock += 1;
  sock = (SocketData*)calloc(num_sock, sizeof(SocketData));
  memcpy(sock, tmp, sizeof(SocketData)*latest_sock);
  free(tmp);

  // create module name from UART_PORT number
  char* module_name = malloc(2*sizeof(char));
  snprintf(module_name, 2, "%d", ((int)uart + 1));

  sock[latest_sock].port = uart;
  sock[latest_sock].storage.context = settings->context;
  sock[latest_sock].storage.rx_handler = settings->rx_handler;

  x86_socket_init(&sock[num_sock-1].thread, module_name, receiver_wrapper, settings->context);

  return STATUS_CODE_OK;
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {

  // check if the port has been initilized
  int current_sock;
  if((current_sock = get_sock(uart)) < 0) {
    LOG_DEBUG("UART Port %d has not been initialized.\n", uart);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  sock[current_sock].storage.rx_handler = rx_handler;
  sock[current_sock].storage.context = context;
  sock[current_sock].thread.context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {

  // check if the port has been initilized
  int current_sock;
  if((current_sock = get_sock(uart)) < 0) {
    LOG_DEBUG("UART Port %d has not been initialized.\n", uart);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  x86_socket_broadcast(&sock[current_sock].thread, (char*)tx_data, len);

  return STATUS_CODE_OK;
}
