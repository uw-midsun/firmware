#include "power_distribution_controller.h"

#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "input_event.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_storage;

typedef struct TestPowerDistributionControllerAckCtx {
  EEPowerState expected_state;
  CANAckStatus returned_status;
} TestPowerDistributionControllerAckCtx;

// Handler that allows for injecting ack responses.
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)msg;
  TestPowerDistributionControllerAckCtx *ctx = context;
  EEPowerState state = NUM_EE_POWER_STATES;
  CAN_UNPACK_POWER_STATE(msg, (uint8_t *)&state);
  *ack_reply = ctx->returned_status;
  TEST_ASSERT_EQUAL(ctx->expected_state, state);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();

  CanSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,  // Pretend to be the target so loopback works.
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_storage, &settings);
}

void teardown_test(void) {}

// Tests state output
void test_emergency_fault(void) {
  TestPowerDistributionControllerAckCtx ctx = {
    .expected_state = EE_POWER_STATE_IDLE,
    .returned_status = CAN_ACK_STATUS_OK,
  };
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_rx_handler, &ctx));

  Event e = { 0 };
  // Send once and ACK.
  power_distribution_controller_send_update(EE_POWER_STATE_IDLE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Fail once then succeed
  ctx.returned_status = CAN_ACK_STATUS_INVALID;
  power_distribution_controller_send_update(EE_POWER_STATE_IDLE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_RETRY_POWER_STATE, e.id);
  TEST_ASSERT_EQUAL(EE_POWER_STATE_IDLE, e.data);
  ctx.returned_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(power_distribution_controller_retry(&e));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Timeout (major issue)
  ctx.returned_status = CAN_ACK_STATUS_TIMEOUT;
  power_distribution_controller_send_update(EE_POWER_STATE_IDLE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(INPUT_EVENT_CAN_TX, INPUT_EVENT_CAN_RX);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_BPS_FAULT, e.id);
}
