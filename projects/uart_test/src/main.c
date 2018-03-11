// x86 UART initiate receiver and transmitter
#include <stdio.h>
#include "log.h"
#include "uart.h"

void receiver_handler(const uint8_t *rx_arr, size_t len, void *context) {
  char arr[len];
  // print out the received output
  for (int i = 0; i < (int)len; i++) {
    arr[i] = (char)*(rx_arr + (uint8_t)i);
  }
  LOG_DEBUG("%s\n", arr);
}

int main(void) {
  // Start of Setup

  // create settings for the UART device - npte: not alt fn on x86
  UARTSettings settings = {
    .baudrate = 115200, .rx_handler = receiver_handler, .context = 0, .alt_fn = GPIO_ALTFN_NONE
  };

  // declare a fifo input and a fifo output buffer
  uint8_t transfer[UART_MAX_BUFFER_LEN] = { 0 };
  uint8_t receiver[UART_MAX_BUFFER_LEN] = { 0 };

  Fifo tx_fifo = {
    .buffer = transfer,
    .end = (transfer + 255),
    .head = transfer,
    .next = (transfer + 1),
    .num_elems = 0,
    .max_elems = UART_MAX_BUFFER_LEN,
    .elem_size = 1,
  };

  Fifo rx_fifo = {
    .buffer = receiver,
    .end = (receiver + 255),
    .head = receiver,
    .next = (receiver + 1),
    .num_elems = 0,
    .max_elems = UART_MAX_BUFFER_LEN,
    .elem_size = 1,
  };

  // create storage struct for the uart device
  UARTStorage storage = {
    .rx_handler = receiver_handler, .context = 0, .tx_fifo = tx_fifo, .rx_fifo = rx_fifo
  };

  LOG_DEBUG("UART Init Status: %d\n", uart_init(UART_PORT_1, &settings, &storage));

  // End of Setup

  // load a string
  uint8_t send[14] = "Hello, World!\n";
  uart_tx(UART_PORT_1, send, 14);

  // receive bytes from tnt1
  while (1) {
    prv_rx_push(UART_PORT_1);
  }

  return 0;
}
