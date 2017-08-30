// CAN <-> UART
#include "translate.h"
#include "can.h"
#include "uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "event_queue.h"
#include "soft_timer.h"
#include "wait.h"

#define TELEMETRY_CAN_DEVICE_ID (CAN_MSG_MAX_DEVICES - 1)
// TODO switch UART port
#define TELEMETRY_UART_PORT UART_PORT_1

static CANStorage s_can_storage = { 0 };
static CANRxHandler s_rx_handlers[1] = { 0 }; // Only need one RX handler
static UARTStorage s_uart_storage = { 0 };

typedef enum {
  TELEMETRY_EVENT_CAN_FAULT = 0,
  TELEMETRY_EVENT_CAN_RX,
  TELEMETRY_EVENT_CAN_TX,
  NUM_TELEMETRY_EVENTS
} TelemetryEvent;

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  UARTSettings uart_settings = {
    .baudrate = 115200,
    .tx = { GPIO_PORT_A, 9 },
    .rx = { GPIO_PORT_A, 10 },
    .alt_fn = GPIO_ALTFN_1
  };
  uart_init(TELEMETRY_UART_PORT, &uart_settings, &s_uart_storage);

  CANSettings can_settings = {
    .device_id = TELEMETRY_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TELEMETRY_EVENT_CAN_RX,
    .tx_event = TELEMETRY_EVENT_CAN_TX,
    .fault_event = TELEMETRY_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };

  can_init(&can_settings, &s_can_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  translate_init(TELEMETRY_UART_PORT);

  while (true) {
    wait();
  }

  return 0;
}