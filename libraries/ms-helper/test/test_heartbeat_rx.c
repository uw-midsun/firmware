#include "heartbeat_rx.h"

#include "can.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_RELAY_CAN_DEVICE_ID 10
#define NUM_TEST_RELAY_RX_CAN_HANDLERS 3
#define NUM_TEST_RELAY_RX_STORAGE_HANDLERS 2

typedef enum {
  TEST_RELAY_RX_CAN_RX = 10,
  TEST_RELAY_RX_CAN_TX,
  TEST_RELAY_RX_CAN_FAULT,
} TestCanEvent;

typedef struct TestRelayRxHandlerCtx {
  StatusCode ret_code;
  SystemCanMessage expected_msg_id;
  EEChaosCmdRelayState expected_state;
  bool executed;
} TestRelayRxHandlerCtx;

static const Event s_tx_event = { TEST_RELAY_RX_CAN_TX, 0 };
static const Event s_rx_event = { TEST_RELAY_RX_CAN_RX, 0 };
static HeartbeatRxHandlerStorage s_hb_storage;
static CANStorage s_can_storage;
static CANAckRequests s_can_ack_requests;
static CANRxHandler s_rx_handlers[NUM_TEST_RELAY_RX_STORAGE_HANDLERS];

// CANAckRequestCb
static StatusCode prv_ack_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)num_remaining;
  CANAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT, msg_id);
  TEST_ASSERT_EQUAL(TEST_RELAY_CAN_DEVICE_ID, device);
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

// RelayRxHandler
static StatusCode prv_relay_rx_handler(SystemCanMessage msg_id, EEChaosCmdRelayState state,
                                       void *context) {
  TestRelayRxHandlerCtx *data = context;
  TEST_ASSERT_EQUAL(data->expected_msg_id, msg_id);
  TEST_ASSERT_EQUAL(data->expected_state, state);
  data->executed = true;
  return data->ret_code;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = TEST_RELAY_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_RELAY_RX_CAN_RX,
    .tx_event = TEST_RELAY_RX_CAN_TX,
    .fault_event = TEST_RELAY_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(
      can_init(&can_settings, &s_can_storage, s_rx_handlers, NUM_TEST_RELAY_RX_CAN_HANDLERS));
  can_ack_init(&s_can_ack_requests);
}

void teardown_test(void) {}

void test_relay_rx_guards(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, relay_rx_init(s_relay_storage, 0));
  TEST_ASSERT_OK(relay_rx_init(s_relay_storage, NUM_TEST_RELAY_RX_STORAGE_HANDLERS));

  TestRelayRxHandlerCtx context = {
    .ret_code = STATUS_CODE_OK,
    .expected_msg_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY,
    .expected_state = EE_CHAOS_CMD_RELAY_STATE_OPEN,
    .executed = false,
  };
  TEST_ASSERT_OK(
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_BATTERY_RELAY, prv_relay_rx_handler, &context));
