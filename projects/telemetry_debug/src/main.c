// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "delay.h"
#include "event_queue.h"
#include "generic_can_uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "uart.h"

int main(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();
  UARTStorage uart_storage = { 0 };
  UARTSettings uart_settings = {
    .baudrate = 115200,
    .rx_handler = NULL,
    .context = NULL,
    .tx = { GPIO_PORT_B, 6 },
    .rx = { GPIO_PORT_B, 7 },
    .alt_fn = GPIO_ALTFN_0,
  };
  uart_init(UART_PORT_1, &uart_settings, &uart_storage);

  GenericCanUart can_uart;
  generic_can_uart_init(&can_uart, UART_PORT_1);
  GenericCan *can = (GenericCan *)&can_uart;

  GenericCanMsg msg = {
    .id = 12,
    .data = 0,
    .dlc = 1,
    .extended = false,
  };
  while (true) {
    generic_can_tx(can, &msg);
    msg.data = (msg.data + 1) % 120;
    delay_s(1);
  }

  return 0;
}
