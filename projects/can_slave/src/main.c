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
#include "debug_led.h"

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
    .loopback = true,                         //
  };

  can_hw_init(&can_hw_settings);
}

#include <stdlib.h>
static void prv_periodic_tick(SoftTimerID timer_id, void *context) {
  static uint32_t s_id = 0;
  uint16_t data[4] = { rand(), rand(), rand(), rand() };
  uint32_t id = s_id++ % 0x800;
  size_t dlc = (size_t)rand() % 9;
  can_hw_transmit(id, false, (uint8_t *)&data, dlc);
  printf("TX 0x%lx (size %d)\n", id, dlc);

  debug_led_toggle_state(DEBUG_LED_RED);

  soft_timer_start_millis(50, prv_periodic_tick, NULL, NULL);
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

  debug_led_init(DEBUG_LED_RED);

  soft_timer_start_millis(50, prv_periodic_tick, NULL, NULL);

  while (true) {
    wait();
  }

  return 0;
}
