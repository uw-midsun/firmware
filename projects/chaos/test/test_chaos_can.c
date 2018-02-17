#include "chaos_can.h"

#include <stddef.h>

#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_NULL_ID UINT16_MAX

static CANAckRequests s_can_ack_requests;

static StatusCode prv_ack_power_state_callback(CANMessageID msg_id, uint16_t device,
                                               CANAckStatus status, uint16_t num_remaining,
                                               void *context) {
  TEST_ASSERT_EQUAL(0, num_remaining);
  TEST_ASSERT_EQUAL(CAN_DEVICE_CHAOS, device);
  TEST_ASSERT_EQUAL(CAN_MESSAGE_POWER_STATE, msg_id);
  CANAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

static StatusCode prv_ack_bps_fault_callback(CANMessageID msg_id, uint16_t device,
                                             CANAckStatus status, uint16_t num_remaining,
                                             void *context) {
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, status);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings settings = {
    .device_id = CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(chaos_can_init(&settings));
  can_ack_init(&s_can_ack_requests);
}

void teardown_test(void) {}

typedef struct TestChaosCanParams {
  ChaosCanPowerState power_state;
  Event expected_output;
  CANAckStatus expected_status;
  size_t expected_can_cnt;
  size_t expected_events_cnt;
} TestChaosCanParams;

// Tests that the power state transitions are handled correctly. Specifically,
// it validates that the correct event is raised in the event each of the power
// state change requests are delivered.
void test_chaos_can_power_state(void) {
  // Setup the ACK handlers callback.
  uint32_t expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
  CANAckStatus expected_status = NUM_ACK_STATUSES;
  const CANAckRequest req = {
    .context = &expected_status,
    .callback = prv_ack_power_state_callback,
    .expected_bitset = expected_bitset,
  };

  // Test parameters:
  //
  // Validate that when |power_state| is received that |expected_output| occurs as an event that the
  // |expected_status| for the ack occurs and that the |expected_*_cnt| type of events occur.
  const TestChaosCanParams params[] = {
    { .power_state = CHAOS_CAN_POWER_STATE_IDLE,
      .expected_output = { .id = CHAOS_EVENT_SEQUENCE_IDLE },
      .expected_can_cnt = 2,
      .expected_events_cnt = 1,
      .expected_status = CAN_ACK_STATUS_OK },
    { .power_state = CHAOS_CAN_POWER_STATE_CHARGE,
      .expected_output = { .id = CHAOS_EVENT_SEQUENCE_CHARGE },
      .expected_can_cnt = 2,
      .expected_events_cnt = 1,
      .expected_status = CAN_ACK_STATUS_OK },
    { .power_state = CHAOS_CAN_POWER_STATE_DRIVE,
      .expected_output = { .id = CHAOS_EVENT_SEQUENCE_DRIVE },
      .expected_can_cnt = 2,
      .expected_events_cnt = 1,
      .expected_status = CAN_ACK_STATUS_OK },
    { .power_state = NUM_CHAOS_CAN_POWER_STATES,
      .expected_output = { .id = TEST_NULL_ID },
      .expected_can_cnt = 2,
      .expected_events_cnt = 0,
      .expected_status = CAN_ACK_STATUS_INVALID },
  };
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;

  for (uint16_t i = 0; i < SIZEOF_ARRAY(params); i++) {
    TEST_ASSERT_OK(CAN_TRANSMIT_POWER_STATE(&req, params[i].power_state));

    // Request
    // TX
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    TEST_ASSERT_TRUE(chaos_can_process_event(&e));
    // RX
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    TEST_ASSERT_TRUE(chaos_can_process_event(&e));

    size_t can_event_cnt = 0;
    size_t output_cnt = 0;

    // Ack and Process (non-deterministic order)
    expected_status = params[i].expected_status;
    for (size_t j = 0; j < params[i].expected_can_cnt + params[i].expected_events_cnt; j++) {
      do {
        status = event_process(&e);
      } while (status != STATUS_CODE_OK);
      if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX) {
        TEST_ASSERT_TRUE(chaos_can_process_event(&e));
        can_event_cnt++;
      } else {
        if (params[i].expected_output.id != TEST_NULL_ID) {
          // Check event
          TEST_ASSERT_EQUAL(params[i].expected_output.id, e.id);
          output_cnt++;
        }
      }
    }
    TEST_ASSERT_EQUAL(params[i].expected_can_cnt, can_event_cnt);
    TEST_ASSERT_EQUAL(params[i].expected_events_cnt, output_cnt);
  }
}

void test_chaos_can_bps_fault(void) {
  // Setup the ACK handlers callback.
  uint32_t expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
  CANAckStatus expected_status = NUM_ACK_STATUSES;
  const CANAckRequest req = {
    .context = &expected_status,
    .callback = prv_ack_bps_fault_callback,
    .expected_bitset = expected_bitset,
  };

  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  TEST_ASSERT_OK(CAN_TRANSMIT_BPS_FAULT(&req));
  // Request
  // TX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(chaos_can_process_event(&e));
  // RX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(chaos_can_process_event(&e));

  // Ack and Process (non-deterministic order)
  while (!(e.id == CHAOS_EVENT_SEQUENCE_EMERGENCY)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);

    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX) {
      TEST_ASSERT_TRUE(chaos_can_process_event(&e));
    }
  }
}
