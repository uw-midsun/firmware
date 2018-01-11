#include "event_queue.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static StatusCode prv_raise_event(uint16_t i) {
  return event_raise(i, i * 100);
}

void setup_test(void) { event_queue_init(); }

void teardown_test(void) {}

void test_event_queue_raise(void) {
  // Fill the event queue
  for (int i = EVENT_QUEUE_SIZE; i > 0; i--) {
    TEST_ASSERT_OK(prv_raise_event(i));
  }

  // Attempt to insert an element when full
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, prv_raise_event(0));

  Event e;
  uint16_t i = 1; // Start at 1 since the insertion of 0 should've failed
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(i, e.id);
    TEST_ASSERT_EQUAL(i * 100, e.data);
    i++;
  }

  TEST_ASSERT_OK(prv_raise_event(5));
  TEST_ASSERT_OK(event_process(&e));

  TEST_ASSERT_EQUAL(5, e.id);
  TEST_ASSERT_EQUAL(5 * 100, e.data);

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
