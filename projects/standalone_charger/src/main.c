// Pretends to be driver controls and power distribution
// - Responds to BPS heartbeats
// - Closes main relays
// - Communicates with the charger
// - Monitors power data
#include "gpio.h"
#include "can.h"
#include "can_hw.h"
#include "event_queue.h"
#include "can_msg_defs.h"
#include "charger_events.h"
#include "generic_can_uart.h"
#include "uart.h"
#include "wait.h"
#include "charger.h"
#include "can_unpack.h"
#include "interrupt.h"
#include "log.h"
#include "can_transmit.h"
#include "delay.h"

// 2.5V * 36 = 151.2V
#define STANDALONE_CHARGER_MAX_VOLTAGE 1512

static CANStorage s_can;
static GenericCanUart s_charger_can;
static UARTStorage s_uart;

static bool s_connected;

static StatusCode prv_handle_heartbeat(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // Respond to BPS heartbeat
  CANAckStatus status = CAN_ACK_STATUS_OK;
  CANId bps_heartbeat_ack = {
    .source_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .type = CAN_MSG_TYPE_ACK,
    .msg_id = SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT,
  };
  can_hw_transmit(bps_heartbeat_ack.raw, false, &status, sizeof(status));

  return STATUS_CODE_OK;
}

static StatusCode prv_handle_battery_vc(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  uint32_t voltage = 0;
  int32_t current = 0;
  CAN_UNPACK_BATTERY_AGGREGATE_VC(msg, &voltage, (uint32_t *)&current);

  // Relays are disconnected if the bus voltage is lower than our bus voltage
  s_connected = voltage / 10000 > 90;

  LOG_DEBUG("Battery: %ld.%ldV, %ld.%ldA\n", voltage / 10000,  voltage % 10000, current / 1000000, current % 1000000);

  return STATUS_CODE_OK;
}

static void prv_charger_info(uint16_t voltage, uint16_t current, ChargerCanStatus status, void *context) {
  LOG_DEBUG("Charger: %d.%dV, %d.%dA (status 0x%x)\n", voltage / 10, voltage % 10, current / 10, current % 10, status.raw);
  CAN_TRANSMIT_CHARGER_INFO(voltage, current, status.raw);
}

int main(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CHARGER_EVENT_CAN_RX,
    .tx_event = CHARGER_EVENT_CAN_TX,
    .fault_event = CHARGER_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

  UARTSettings uart_settings = {
    .baudrate = 115200,
    .rx_handler = NULL,
    .context = NULL,
    .tx = { GPIO_PORT_B, 10 },
    .rx = { GPIO_PORT_B, 11 },
    .alt_fn = GPIO_ALTFN_4,
  };
  uart_init(UART_PORT_3, &uart_settings, &s_uart);
  generic_can_uart_init(&s_charger_can, UART_PORT_3);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_handle_heartbeat, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, prv_handle_battery_vc, NULL);

  ChargerSettings charger_settings = {
    .charger_can = (GenericCan *)&s_charger_can,
    .info_cb = prv_charger_info,
    .context = NULL,
  };
  charger_init(&charger_settings);

  delay_s(3);

  charger_start(1512, 100);

  LOG_DEBUG("hello\n");

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }

    wait();
  }

  return 0;
}