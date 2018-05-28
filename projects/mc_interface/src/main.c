#include "can.h"
#include "can_msg_defs.h"
#include "drive_can.h"
#include "generic_can_uart.h"
#include "gpio.h"
#include "interrupt.h"
#include "mc_cfg.h"
#include "motor_controller.h"
#include "uart.h"
#include "wait.h"

typedef enum {
  MOTOR_EVENT_SYSTEM_CAN_RX = 0,
  MOTOR_EVENT_SYSTEM_CAN_TX,
  MOTOR_EVENT_SYSTEM_CAN_FAULT,
} MotorEvent;

static MotorControllerStorage s_controller_storage;
static GenericCanUart s_can_uart;
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[MC_CFG_NUM_CAN_RX_HANDLERS];
static UARTStorage s_uart_storage;

static void prv_setup_system_can(void) {
  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = MC_CFG_CAN_BITRATE,
    .rx_event = MOTOR_EVENT_SYSTEM_CAN_RX,
    .tx_event = MOTOR_EVENT_SYSTEM_CAN_TX,
    .fault_event = MOTOR_EVENT_SYSTEM_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&can_settings, &s_can_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
}

static void prv_setup_motor_can(void) {
  UARTSettings uart_settings = {
    .baudrate = MC_CFG_CAN_UART_BAUDRATE,
    .tx = MC_CFG_CAN_UART_TX,
    .rx = MC_CFG_CAN_UART_RX,
    .alt_fn = MC_CFG_CAN_UART_ALTFN,
  };
  uart_init(MC_CFG_CAN_UART_PORT, &uart_settings, &s_uart_storage);

  generic_can_uart_init(&s_can_uart, MC_CFG_CAN_UART_PORT);
}

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();
  prv_setup_system_can();
  prv_setup_motor_can();

  // clang-format off
  MotorControllerSettings mc_settings = {
    .can_uart = (GenericCan *)&s_can_uart,
    .ids = {
      [MOTOR_CONTROLLER_LEFT] = {
          .motor_controller = MC_CFG_MOTOR_CAN_ID_MC_LEFT,
          .interface = MC_CFG_MOTOR_CAN_ID_DC_LEFT,
      },
      [MOTOR_CONTROLLER_RIGHT] = {
          .motor_controller = MC_CFG_MOTOR_CAN_ID_MC_RIGHT,
          .interface = MC_CFG_MOTOR_CAN_ID_DC_RIGHT,
      },
    },
    .max_bus_current = MC_CFG_MOTOR_MAX_BUS_CURRENT,
  };
  // clang-format on

  motor_controller_init(&s_controller_storage, &mc_settings);
  drive_can_init(&s_controller_storage);

  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      fsm_process_event(CAN_FSM, &e);
    }

    wait();
  }

  return 0;
}
