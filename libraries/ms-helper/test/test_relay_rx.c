#include "relay_rx.h"

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_RELAY_CAN_DEVICE_ID 10
#define NUM_TEST_RELAY_RX_CAN_HANDLERS 2
#define NUM_TEST_RELAY_RX_STORAGE_HANDLERS 3

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

static RelayRxStorage s_relay_storage[NUM_TEST_RELAY_RX_STORAGE_HANDLERS];
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[NUM_TEST_RELAY_RX_STORAGE_HANDLERS];

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
      can_init(&can_settings, &s_can_storage, s_rx_handlers, NUM_TEST_RELAY_RX_STORAGE_HANDLERS));
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
  // Fail to register duplicates.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_RESOURCE_EXHAUSTED,
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_BATTERY_RELAY, prv_relay_rx_handler, &context));
  TEST_ASSERT_OK(
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_MAIN_RELAY, prv_relay_rx_handler, &context));

  // Fail to register too many.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_RESOURCE_EXHAUSTED,
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_MAIN_RELAY, prv_relay_rx_handler, &context));
}

void test_relay_rx(void) {
  TEST_ASSERT_OK(relay_rx_init(s_relay_storage, NUM_TEST_RELAY_RX_STORAGE_HANDLERS));
  TestRelayRxHandlerCtx context = {
    .ret_code = STATUS_CODE_OK,
    .expected_msg_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY,
    .expected_state = EE_CHAOS_CMD_RELAY_STATE_OPEN,
    .executed = false,
  };
  TEST_ASSERT_OK(
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_BATTERY_RELAY, prv_relay_rx_handler, &context));
  CAN_TRANSMIT
}
