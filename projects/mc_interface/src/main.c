#include "can.h"
#include "can_msg_defs.h"
#include "drive_can.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
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
static GenericCanMcp2515 s_can_mcp2515;
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[MC_CFG_NUM_CAN_RX_HANDLERS];
static UARTStorage s_uart_storage;
static SequencedRelayStorage s_relay_storage;

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

  can_init(&s_can_storage, &can_settings, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
}

static void prv_setup_motor_can(void) {
  const Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_1,
    .baudrate = 750000,
    .mosi = { .port = GPIO_PORT_A, 7 },
    .miso = { .port = GPIO_PORT_A, 6 },
    .sclk = { .port = GPIO_PORT_A, 5 },
    .cs = { .port = GPIO_PORT_A, 4 },
    .int_pin = { .port = GPIO_PORT_A, 3 },
    .loopback = false,
  };
  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
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
    .motor_can = (GenericCan *)&s_can_mcp2515,
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

  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      fsm_process_event(CAN_FSM, &e);
    }

    wait();
  }

  return 0;
}
