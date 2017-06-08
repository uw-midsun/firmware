#include "timer.h"
#include "unity.h"
#include "test_helpers.h"
#include "log.h"

static void prv_timeout_cb(SoftTimerID timer_id, void *context) {
  SoftTimerID *cb_id = context;
  *cb_id = timer_id;
}

void setup_test(void) {
  interrupt_init();
  timer_init();
}

void teardown_test(void) { }

void test_timer_basic(void) {
  volatile SoftTimerID cb_id = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(timer_inuse());

  StatusCode ret;
  ret = timer_start_ms(1, prv_timeout_cb, &cb_id, &id);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id);
  TEST_ASSERT_TRUE(timer_inuse());

  while (cb_id == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id, cb_id);
  TEST_ASSERT_FALSE(timer_inuse());
}
