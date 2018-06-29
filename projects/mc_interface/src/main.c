#include "can.h"
#include "can_msg_defs.h"
#include "drive_can.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "heartbeat_rx.h"
#include "interrupt.h"
#include "mc_cfg.h"
#include "mcp2515.h"
#include "motor_controller.h"
#include "sequenced_relay.h"
#include "wait.h"

typedef enum {
  MOTOR_EVENT_SYSTEM_CAN_RX = 0,
  MOTOR_EVENT_SYSTEM_CAN_TX,
  MOTOR_EVENT_SYSTEM_CAN_FAULT,
} MotorEvent;

static MotorControllerStorage s_controller_storage;
static GenericCanMcp2515 s_can_mcp2515;
static CANStorage s_can_storage;
static SequencedRelayStorage s_relay_storage;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

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

  can_init(&s_can_storage, &can_settings);
}

static void prv_setup_motor_can(void) {
  Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .baudrate = 750000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },
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
