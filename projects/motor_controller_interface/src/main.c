#include <stdbool.h>
#include <stdlib.h>

#include "can_uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"

#include "can.h"
#include "can_interval.h"
#include "can_msg_defs.h"
#include "generic_can_uart.h"
#include "motor_controller.h"

#include "config.h"
#include "motor_controller_fsm.h"
#include "motor_controller_interface_events.h"

static UARTStorage s_uart_storage;
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[MOTOR_CONTROLLER_INTERFACE_CAN_RX_NUM_HANDLERS];

static void prv_telemetry_callback(SoftTimerID timer_id, void *context) {}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX,
    .tx_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX,
    .fault_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&can_settings, &s_can_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));

  // Update the software timer
  MotorControllerMeasurement mc_measurements = { 0 };

  UARTSettings uart_settings = {
    .baudrate = 115200,                       //
    .tx = { .port = GPIO_PORT_A, .pin = 3 },  //
    .rx = { .port = GPIO_PORT_A, .pin = 2 },  //
    .alt_fn = GPIO_ALTFN_1,                   //
  };
  uart_init(UART_PORT_2, &uart_settings, &s_uart_storage);

  GenericCanUart can_uart = { 0 };
  generic_can_uart_init(&can_uart, UART_PORT_2);

  can_interval_init();
  Fifo fifo;
  MotorControllerFsmStorage mc_fsm_storage = {
    .generic_can = (GenericCan *)&can_uart,
    .measurement = &mc_measurements,
    .fifo = &fifo,
  };
  motor_controller_fsm_init(&mc_fsm_storage);

  // Register a software timer for telemetry
  SoftTimerID telemetry_id = SOFT_TIMER_INVALID_TIMER;
  soft_timer_start_millis(50, prv_telemetry_callback, &mc_measurements, &telemetry_id);

  Event e = { 0 };
  while (true) {
    // Process event messages sent from internal CAN bus and then pass through
    // via CanUart can_uart_req_slave_tx
    while (status_ok(event_process(&e))) {
      // We want to transition the state machine on each RX event
      if (e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX) {
        motor_controller_fsm_process_event(&e);
      }
    }
  }

  return 0;
}
