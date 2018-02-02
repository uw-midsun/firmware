#include <stdbool.h>
#include <stddef.h>

#include "can_hw.h"
#include "can_uart.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "uart.h"
#include "wait.h"

static UARTStorage s_uart_storage;

static void prv_init_periph(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CANHwSettings can_hw_settings = {
    .tx = { .port = GPIO_PORT_A, .pin = 12 },  //
    .rx = { .port = GPIO_PORT_A, .pin = 11 },  //
    .bitrate = CAN_HW_BITRATE_500KBPS,         //
    .loopback = false,                         //
  };

  can_hw_init(&can_hw_settings);

  UARTSettings uart_settings = {
    .baudrate = 115200,                       //
    .tx = { .port = GPIO_PORT_A, .pin = 3 },  //
    .rx = { .port = GPIO_PORT_A, .pin = 2 },  //
    .alt_fn = GPIO_ALTFN_1                    //
  };
  uart_init(UART_PORT_2, &uart_settings, &s_uart_storage);
}

int main(void) {
  prv_init_periph();

  CanUart can_uart = {
    .uart = UART_PORT_2,  //
    .rx_cb = NULL,        // Ignore RX'd messages from the master
    .context = NULL       //
  };
  can_uart_init(&can_uart);
  can_uart_enable_passthrough(&can_uart);

  GPIOAddress led = { .port = GPIO_PORT_A, .pin = 15 };
  GPIOSettings settings = { .direction = GPIO_DIR_OUT, .state = GPIO_STATE_LOW };
  gpio_init_pin(&led, &settings);

  while (true) {
    delay_s(1);
    gpio_toggle_state(&led);
  }

  return 0;
}
