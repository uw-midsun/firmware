#include <stdbool.h>
#include "can_hw.h"
#include "can_uart.h"
#include "uart.h"
#include "wait.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "delay.h"

static UARTStorage s_uart_storage;

static void prv_init_periph(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CANHwSettings can_hw_settings = {
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .loopback = false,
  };

  can_hw_init(&can_hw_settings);

  UARTSettings uart_settings = {
    .baudrate = 115200, //
    // .tx = { GPIO_PORT_A, 3 }, //
    // .rx = { GPIO_PORT_A, 2 }, //
    // .alt_fn = GPIO_ALTFN_1 //
    .tx = { GPIO_PORT_B, 6 },
    .rx = { GPIO_PORT_B, 7 },
    .alt_fn = GPIO_ALTFN_0
  };
  uart_init(UART_PORT_1, &uart_settings, &s_uart_storage);
}

int main(void) {
  prv_init_periph();

  CanUart can_uart = {
    .uart = UART_PORT_1, //
    .rx_cb = NULL,  // Ignore RX'd messages from the master
    .context = NULL //
  };
  can_uart_init(&can_uart);
  can_uart_hook_can_hw(&can_uart);

  GPIOAddress led = { .port = GPIO_PORT_A, .pin = 15 };
  GPIOSettings settings = { .direction = GPIO_DIR_OUT, .state = GPIO_STATE_LOW };
  gpio_init_pin(&led, &settings);

  while (true) {
    delay_s(1);
    gpio_toggle_state(&led);
    const char *test = "hello world\n";
    // uart_tx(UART_PORT_1, test, 12);
  }

  return 0;
}
