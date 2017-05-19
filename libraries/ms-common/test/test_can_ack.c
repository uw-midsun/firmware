#include "can_ack.h"
#include "unity.h"
#include "test_helpers.h"

static CANAckRequests s_ack_requests;

typedef struct TestResponse {
  CANMessageID msg_id;
  uint16_t device;
  CANAckStatus response;
  uint16_t num_remaining;
} TestResponse;

static StatusCode prv_ack_callback(CANMessageID msg_id, uint16_t device, CANAckStatus response,
                                   uint16_t num_remaining, void *context) {
  printf("Expired msg %d (%d remaining)\n", msg_id, num_remaining);
  TestResponse *data = context;
  data->msg_id = msg_id;
  data->device = device;
  data->response = response;
  data->num_remaining = num_remaining;
}

void setup_test(void) {
  can_ack_init(&s_ack_requests);
}

void teardown_test(void) { }

void test_check_expiry(void) {
  TestResponse data = { 0 };
  can_ack_add_request(&s_ack_requests, 0x4, 1, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x2, 2, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x3, 3, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x2, 4, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x1, 5, prv_ack_callback, &data);
  can_ack_add_request(&s_ack_requests, 0x2, 6, prv_ack_callback, &data);

  // Expire the 2nd 0x2 message, then any 0x2 message
  CANAckRequest ack_request = { .msg_id = 0x2 };
  for (size_t i = 0; i < s_ack_requests.num_requests; i++) {
    if (s_ack_requests.active_requests[i]->num_remaining == 4) {
      ack_request.timer = s_ack_requests.active_requests[i]->timer;
    }
  }
  TEST_ASSERT_NOT_EQUAL(0, ack_request.timer);

  printf("Expiring message ID %d, timer %d\n", ack_request.msg_id, ack_request.timer);
  StatusCode ret = can_ack_expire(&s_ack_requests, &ack_request);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(ack_request.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(4, data.num_remaining);

  // Should remove the first message with ID 0x2
  ack_request.timer = 0;
  ret = can_ack_expire(&s_ack_requests, &ack_request);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(ack_request.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(2, data.num_remaining);

  printf("Expiring all remaining %d messages\n", s_ack_requests.num_requests);
  while (s_ack_requests.num_requests > 0) {
    uint16_t num_remaining = s_ack_requests.active_requests[0]->num_remaining;
    ret = can_ack_expire(&s_ack_requests, s_ack_requests.active_requests[0]);
    TEST_ASSERT_OK(ret);
    TEST_ASSERT_EQUAL(num_remaining, data.num_remaining);
  }
}
