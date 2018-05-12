#include "relay_rx.h"

#include <stdbool.h>

#include "can.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_TEST_RELAY_RX_CAN_HANDLERS 2
#define NUM_TEST_RELAY_RX_STORAGE_HANDLERS 1

static RelayRxStorage s_relay_storage[NUM_TEST_RELAY_RX_STORAGE_HANDLERS];

typedef struct TestRelayRxHandlerCtx {
  StatusCode ret_code;
  SystemCanMessage expected_msg_id;
  EEChaosCmdRelayState expected_state;
  bool executed;
} TestRelayRxHandlerCtx;

static StatusCode prv_relay_rx_handler(SystemCanMessage msg_id, EEChaosCmdRelayState state,
                                       void *context) {
  TestRelayRxHandlerCtx *data = context;
  TEST_ASSERT_EQUAL(data->expected_msg_id, msg_id);
  TEST_ASSERT_EQUAL(data->expected_state, state);
  data->executed = true;
  return data->ret_code;
}

void setup_test(void) {
  TEST_ASSERT_OK(relay_rx_init(s_relay_storage, NUM_TEST_RELAY_RX_STORAGE_HANDLERS));
}

void teardown_test(void) {}

void test_relay_rx(void) {
  const SystemCanMessage test_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY;
  const EEChaosCmdRelayState relay_state = NUM_EE_CHAOS_CMD_RELAY_STATES;
  TestRelayRxHandlerCtx context = {
    .ret_code = STATUS_CODE_OK,
    .expected_msg_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY,
    .expected_state = EE_CHAOS_CMD_RELAY_STATE_OPEN,
    .executed = false,
  };
  TEST_ASSERT_OK(
      relay_rx_configure_handler(SYSTEM_CAN_MESSAGE_BATTERY_RELAY, prv_relay_rx_handler, ));
}
