#include <stdbool.h>
#include "can_hw.h"
#include "can_uart.h"
#include "uart.h"
#include "wait.h"

static UARTStorage s_uart_storage;

static void prv_init_periph(void) {
  CANHwSettings can_hw_settings = {
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .bitrate = CAN_HW_BITRATE_1000KBPS,
    .loopback = false,
  };

  can_hw_init(&can_hw_settings);

  UARTSettings uart_settings = {
    .baudrate = 115200, .tx = { GPIO_PORT_A, 3 }, .rx = { GPIO_PORT_A, 2 }, .alt_fn = GPIO_ALTFN_1
  };
  uart_init(UART_PORT_2, &uart_settings, &s_uart_storage);
}

int main(void) {
  prv_init_periph();

  CanUart can_uart = { .uart = UART_PORT_2,
                       .rx_cb = NULL,  // Ignore RX'd messages from the master
                       .context = NULL };
  can_uart_init(&can_uart);
  can_uart_hook_can_hw(&can_uart);

  while (true) {
    wait();
  }

  return 0;
}
