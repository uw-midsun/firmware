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
#define CAN_SLAVE_UART_PORT UART_PORT_3
#define CAN_SLAVE_UART_TX \
  { .port = GPIO_PORT_B, .pin = 10 }
#define CAN_SLAVE_UART_RX \
  { .port = GPIO_PORT_B, .pin = 11 }
#define CAN_SLAVE_UART_ALTFN GPIO_ALTFN_4
#define CAN_SLAVE_CAN_BITRATE CAN_HW_BITRATE_500KBPS

static UARTStorage s_uart_storage;

static void prv_init_periph(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  UARTSettings uart_settings = {
    .baudrate = CAN_SLAVE_UART_BAUDRATE,  //
    .tx = CAN_SLAVE_UART_TX,              //
    .rx = CAN_SLAVE_UART_RX,              //
    .alt_fn = CAN_SLAVE_UART_ALTFN        //
  };
  uart_init(CAN_SLAVE_UART_PORT, &uart_settings, &s_uart_storage);

  CANHwSettings can_hw_settings = {
    .tx = { .port = GPIO_PORT_A, .pin = 12 },  //
    .rx = { .port = GPIO_PORT_A, .pin = 11 },  //
    .bitrate = CAN_SLAVE_CAN_BITRATE,          //
    .loopback = false,                         //
  };

  can_hw_init(&can_hw_settings);
}

static void prv_rx_cb(const struct CanUart *can_uart, uint32_t id, bool extended,
                      const uint64_t *data, size_t dlc, void *context) {
  LOG_DEBUG("RX id %ld\n", id);
}

int main(void) {
  prv_init_periph();

  CanUart can_uart = {
    .uart = CAN_SLAVE_UART_PORT,  //
    .rx_cb = prv_rx_cb,           // Ignore RX'd messages from the master
    .context = NULL               //
  };
  can_uart_init(&can_uart);
  // can_uart_enable_passthrough(&can_uart);

  while (true) {
    wait();
  }

  return 0;
}
