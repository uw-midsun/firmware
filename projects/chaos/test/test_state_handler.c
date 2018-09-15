#include "state_handler.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static bool s_mock_return = true;

static CanStorage s_can_storage;

// CanAckRequestCb
static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)device;
  (void)num_remaining;
  CanAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_POWER_STATE, msg_id);
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
}

void teardown_test(void) {}

void test_state_handler(void) {
  TEST_ASSERT_OK(state_handler_init());
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;
  CANAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS),
  };

  Event e = { 0, 0 };
  // Successful sends (valid transition).

  // IDLE
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_IDLE);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_IDLE, e.id);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);

  // CHARGE
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_CHARGE);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_CHARGE, e.id);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);

  // DRIVE
  CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_DRIVE);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_DRIVE, e.id);
  MS_TEST_HELPER_CAN_TX_RX(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);

  // Invalid sends.
  expected_status = CAN_ACK_STATUS_INVALID;
  CAN_TRANSMIT_POWER_STATE(&req, NUM_EE_POWER_STATES);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
