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

#define CAN_SLAVE_UART_BAUDRATE 115200
#define CAN_SLAVE_UART_PORT UART_PORT_2
#define CAN_SLAVE_UART_TX \
  { .port = GPIO_PORT_A, .pin = 2 }
#define CAN_SLAVE_UART_RX \
  { .port = GPIO_PORT_A, .pin = 3 }
#define CAN_SLAVE_UART_ALTFN GPIO_ALTFN_1
#define CAN_SLAVE_CAN_BITRATE CAN_HW_BITRATE_500KBPS

static UartStorage s_uart_storage;


static const GpioAddress REG_EN = {
  .port = GPIO_PORT_B, .pin = 12
}; 

static const GpioAddress FAN_EN = {
  .port = GPIO_PORT_B, .pin = 9
}; 

static const GpioAddress DISPLAY_EN = {
  .port = GPIO_PORT_B, .pin = 8
}; 

static void prv_init_periph(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  UartSettings uart_settings = {
    .baudrate = CAN_SLAVE_UART_BAUDRATE,  //
    .tx = CAN_SLAVE_UART_TX,              //
    .rx = CAN_SLAVE_UART_RX,              //
    .alt_fn = CAN_SLAVE_UART_ALTFN        //
  };
  uart_init(CAN_SLAVE_UART_PORT, &uart_settings, &s_uart_storage);

  CanHwSettings can_hw_settings = {
    .tx = { .port = GPIO_PORT_A, .pin = 12 },  //
    .rx = { .port = GPIO_PORT_A, .pin = 11 },  //
    .bitrate = CAN_SLAVE_CAN_BITRATE,          //
    .loopback = false,                         //
  };

  can_hw_init(&can_hw_settings);

    GpioSettings pin_settings = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  gpio_init_pin(&REG_EN, &pin_settings);
  gpio_init_pin(&FAN_EN, &pin_settings);
  gpio_init_pin(&DISPLAY_EN, &pin_settings);
  
}

int main(void) {
  prv_init_periph();

  CanUart can_uart = {
    .uart = CAN_SLAVE_UART_PORT,  //
    .rx_cb = NULL,                // Ignore RX'd messages from the master
    .context = NULL               //
  };
  can_uart_init(&can_uart);
  can_uart_enable_passthrough(&can_uart);

  while (true) {
    wait();
  }

  return 0;
}
