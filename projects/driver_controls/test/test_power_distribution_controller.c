#include "power_distribution_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
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

#define NUM_TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_HANDLERS 1

typedef enum {
  TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_RX = 10,
  TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_TX,
  TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_FAULT,
} TestCanEvent;

typedef struct TestPowerDistributionControllerCtx {
  CANMessageID expected_id;
  EEPowerState expected_payload;
  CANAckStatus response_status;
  bool executed;
} TestPowerDistributionControllerCtx;

static const Event s_tx_event = { TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_TX, 0 };
static const Event s_rx_event = { TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_RX, 0 };
static CANStorage s_can_storage;
static CANAckRequests s_can_ack_requests;
static CANRxHandler s_rx_handlers[NUM_TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_HANDLERS];

// CANRxHandler
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  TestPowerDistributionControllerCtx *ctx = context;
  TEST_ASSERT_EQUAL(ctx->expected_id, msg->msg_id);
  EEPowerState state = NUM_EE_POWER_STATES;
  CAN_UNPACK_POWER_STATE(msg, (uint8_t *)&state);
  TEST_ASSERT_EQUAL(ctx->expected_payload, state);
  *ack_reply = ctx->response_status;
  ctx->executed = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,  // Pretend to be Chaos so loopback Ack works.
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .rx_event = TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_RX,
    .tx_event = TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_TX,
    .fault_event = TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&can_settings, &s_can_storage, s_rx_handlers,
                          NUM_TEST_POWER_DISTRIBUTION_CONTROLLER_CAN_HANDLERS));
  can_ack_init(&s_can_ack_requests);
}

void teardown_test(void) {}

void test_power_distribution_controller(void) {
  TestPowerDistributionControllerCtx ctx = {
    .expected_id = SYSTEM_CAN_MESSAGE_POWER_STATE,
    .expected_payload = EE_POWER_STATE_IDLE,
    .response_status = CAN_ACK_STATUS_OK,
    .executed = false,
  };
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_rx_handler, &ctx));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    power_distribution_controller_send_update(NUM_EE_POWER_STATES));

  TEST_ASSERT_OK(power_distribution_controller_send_update(EE_POWER_STATE_IDLE));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  TEST_ASSERT_TRUE(ctx.executed);
  ctx.executed = false;
  ctx.expected_payload = EE_POWER_STATE_CHARGE;

  TEST_ASSERT_OK(power_distribution_controller_send_update(EE_POWER_STATE_CHARGE));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  TEST_ASSERT_TRUE(ctx.executed);
  ctx.executed = false;
  ctx.expected_payload = EE_POWER_STATE_DRIVE;

  TEST_ASSERT_OK(power_distribution_controller_send_update(EE_POWER_STATE_DRIVE));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(s_tx_event, s_rx_event);
  TEST_ASSERT_TRUE(ctx.executed);
}
