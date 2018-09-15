#include "can.h"
#include "can_msg_defs.h"
#include "drive_can.h"
#include "generic_can_uart.h"
#include "gpio.h"
#include "heartbeat_rx.h"
#include "interrupt.h"
#include "mc_cfg.h"
#include "motor_controller.h"
#include "sequenced_relay.h"
#include "uart.h"
#include "wait.h"

typedef enum {
  MOTOR_EVENT_SYSTEM_CAN_RX = 0,
  MOTOR_EVENT_SYSTEM_CAN_TX,
  MOTOR_EVENT_SYSTEM_CAN_FAULT,
} MotorEvent;

static MotorControllerStorage s_controller_storage;
static GenericCanUart s_can_uart;
static CanStorage s_can_storage;
static SequencedRelayStorage s_relay_storage;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;
static UartStorage s_uart_storage;

static void prv_setup_system_can(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
    .bitrate = MC_CFG_CAN_BITRATE,
    .rx_event = MOTOR_EVENT_SYSTEM_CAN_RX,
    .tx_event = MOTOR_EVENT_SYSTEM_CAN_TX,
    .fault_event = MOTOR_EVENT_SYSTEM_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
}

static void prv_setup_motor_can(void) {
  UartSettings uart_settings = {
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
    .motor_can = (GenericCan *)&s_can_uart,
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

  SequencedRelaySettings relay_settings = {
    .can_msg_id = SYSTEM_CAN_MESSAGE_MOTOR_RELAY,
    .left_relay = MC_CFG_RELAY_LEFT,
    .right_relay = MC_CFG_RELAY_RIGHT,
    .delay_ms = MC_CFG_RELAY_DELAY_MS,
  };

  sequenced_relay_init(&s_relay_storage, &relay_settings);

  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }

    wait();
  }

  return 0;
}
