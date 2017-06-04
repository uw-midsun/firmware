#include "can_ack.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "unity.h"
#include "test_helpers.h"

#define TEST_CAN_ACK_INVALID_DEVICE 10

static CANAckRequests s_ack_requests;

typedef struct TestResponse {
  CANMessageID msg_id;
  uint16_t device;
  CANAckStatus status;
  uint16_t num_remaining;
} TestResponse;

static StatusCode prv_ack_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                   uint16_t num_remaining, void *context) {
  LOG_DEBUG("ACK handled: status %d from %d (msg %d) (%d remaining)\n", status, device, msg_id, num_remaining);
  TestResponse *data = context;
  data->msg_id = msg_id;
  data->device = device;
  data->status = status;
  data->num_remaining = num_remaining;

  if (device == TEST_CAN_ACK_INVALID_DEVICE) {
    LOG_DEBUG("Returning unknown code\n");
    return status_code(STATUS_CODE_UNKNOWN);
  }

  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  can_ack_init(&s_ack_requests);
}

void teardown_test(void) { }

void test_can_ack_handle_devices(void) {
  TestResponse data = { 0 };
  CANId can_id = {
    .source_id = 0,
    .type = CAN_MSG_TYPE_ACK,
    .msg_id = 0
  };

  can_ack_add_request(&s_ack_requests, 0x4, 6, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x2, 3, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x6, 6, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x2, 1, prv_ack_callback, &data);

  can_id.msg_id = 0x2;
  LOG_DEBUG("Handling ACK for ID %d, device %d\n", can_id.msg_id, can_id.source_id);
  StatusCode ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_OK(ret);

  // Expect to update the 1st 0x2 ACK request
  TEST_ASSERT_EQUAL(can_id.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(2, data.num_remaining);

  LOG_DEBUG("Handling duplicate ACK\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_OK(ret);

  // Should've updated the 2nd 0x2 ACK request
  TEST_ASSERT_EQUAL(can_id.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(0, data.num_remaining);

  // 3rd duplicate 0x2 ACK should fail
  LOG_DEBUG("Handling duplicate ACK 2\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_EQUAL(STATUS_CODE_UNKNOWN, ret);

  // Valid ACK (new device)
  can_id.source_id = 1;
  LOG_DEBUG("Handling ACK for ID %d, device %d\n", can_id.msg_id, can_id.source_id);
  ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(can_id.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(1, data.num_remaining);

  // Send invalid device ACK - should be able to send another ACK
  can_id.source_id = TEST_CAN_ACK_INVALID_DEVICE;
  LOG_DEBUG("Handling ACK from invalid device\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_OK(ret);

  can_id.source_id = 2;
  LOG_DEBUG("Handling ACK from valid device\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_id);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(0, data.num_remaining);

  // 2 ACK requests should be removed
  TEST_ASSERT_EQUAL(2, s_ack_requests.num_requests);
}

void test_can_ack_expiry(void) {
  volatile TestResponse data = { 0 };
  CANId can_id = {
    .source_id = 0,
    .type = CAN_MSG_TYPE_ACK,
    .msg_id = 0
  };

  can_ack_add_request(&s_ack_requests, 0x2, 5, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x1, 5, prv_ack_callback, &data);

  while (data.msg_id == 0) { }

  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, data.status);
}

// void test_check_expire(void) {
//   TestResponse data = { 0 };
//   can_ack_add_request(&s_ack_requests, 0x4, 1, prv_ack_callback, &data);
//   can_ack_add_request(&s_ack_requests, 0x2, 2, prv_ack_callback, &data);
//   can_ack_add_request(&s_ack_requests, 0x3, 3, prv_ack_callback, &data);
//   can_ack_add_request(&s_ack_requests, 0x2, 4, prv_ack_callback, &data);
//   can_ack_add_request(&s_ack_requests, 0x1, 5, prv_ack_callback, &data);
//   can_ack_add_request(&s_ack_requests, 0x2, 6, prv_ack_callback, &data);

//   // Expire the 2nd 0x2 message, then any 0x2 message
//   CANAckRequest ack_request = { .msg_id = 0x2 };
//   for (size_t i = 0; i < s_ack_requests.num_requests; i++) {
//     if (s_ack_requests.active_requests[i]->num_remaining == 4) {
//       ack_request.timer = s_ack_requests.active_requests[i]->timer;
//     }
//   }
//   TEST_ASSERT_NOT_EQUAL(0, ack_request.timer);

//   printf("Expiring message ID %d, timer %d\n", ack_request.msg_id, ack_request.timer);
//   StatusCode ret = can_ack_expire(&s_ack_requests, &ack_request);
//   TEST_ASSERT_OK(ret);

//   TEST_ASSERT_EQUAL(ack_request.msg_id, data.msg_id);
//   TEST_ASSERT_EQUAL(4, data.num_remaining);

//   // Should remove the first message with ID 0x2
//   ack_request.timer = 0;
//   ret = can_ack_expire(&s_ack_requests, &ack_request);
//   TEST_ASSERT_OK(ret);

//   TEST_ASSERT_EQUAL(ack_request.msg_id, data.msg_id);
//   TEST_ASSERT_EQUAL(2, data.num_remaining);

//   printf("Expiring all remaining %d messages\n", s_ack_requests.num_requests);
//   while (s_ack_requests.num_requests > 0) {
//     uint16_t num_remaining = s_ack_requests.active_requests[0]->num_remaining;
//     ret = can_ack_expire(&s_ack_requests, s_ack_requests.active_requests[0]);
//     TEST_ASSERT_OK(ret);
//     TEST_ASSERT_EQUAL(num_remaining, data.num_remaining);
//   }
// }
