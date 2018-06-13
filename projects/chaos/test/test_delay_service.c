#include "delay_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_delay_service(void) {
  Event e = { CHAOS_EVENT_DELAY_MS, 100 };
  delay_service_process_event(&e);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_DELAY_DONE, e.id);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_delay_service_cancel(void) {
  Event e = { CHAOS_EVENT_DELAY_MS, 10000 };
  delay_service_process_event(&e);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
  test_delay_service_cancel();
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_DELAY_DONE, e.id);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
