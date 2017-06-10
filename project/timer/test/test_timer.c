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

  // Purposely start close to the rollover
  _test_soft_timer_set_counter(UINT32_MAX - 1000);
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

void test_timer_preempt(void) {
  // Begin medium, longer, long, then short - should finish short, medium, long, longer
  // Tests ordering: new, append to back, insert into middle, insert into front
  volatile SoftTimerID cb_id_short = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_medium = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_long = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_longer = SOFT_TIMER_INVALID_TIMER;

  SoftTimerID id_short = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_medium = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_long = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_longer = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(timer_inuse());

  StatusCode ret;

  ret = timer_start_ms(100, prv_timeout_cb, &cb_id_medium, &id_medium);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_medium);

  ret = timer_start_secs(1, prv_timeout_cb, &cb_id_longer, &id_longer);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_longer);

  ret = timer_start_ms(500, prv_timeout_cb, &cb_id_long, &id_long);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_long);

  ret = timer_start(500, prv_timeout_cb, &cb_id_short, &id_short);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_short);

  TEST_ASSERT_TRUE(timer_inuse());

  while (cb_id_short == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id_short, cb_id_short);
  TEST_ASSERT_NOT_EQUAL(id_medium, cb_id_medium);
  TEST_ASSERT_NOT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_medium == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id_medium, cb_id_medium);
  TEST_ASSERT_NOT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_long == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_longer == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id_longer, cb_id_longer);

  TEST_ASSERT_FALSE(timer_inuse());
}

void test_timer_insert_immediate(void) {
  volatile SoftTimerID cb_id = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(timer_inuse());

  StatusCode ret;
  ret = timer_start(1, prv_timeout_cb, &cb_id, &id);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(id, cb_id);

  TEST_ASSERT_FALSE(timer_inuse());
}

void test_timer_cancelled_timer(void) {
  volatile SoftTimerID cb_id_short = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_long = SOFT_TIMER_INVALID_TIMER;

  SoftTimerID id_short = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_long = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(timer_inuse());

  StatusCode ret;
  ret = timer_start_ms(1, prv_timeout_cb, &cb_id_long, &id_long);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_long);

  ret = timer_start(10, prv_timeout_cb, &cb_id_short, &id_short);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_short);

  timer_cancel(id_short);

  while (cb_id_long == SOFT_TIMER_INVALID_TIMER) { }

  TEST_ASSERT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_short, cb_id_short);

  TEST_ASSERT_FALSE(timer_inuse());
}

void test_timer_remaining(void) {
  volatile SoftTimerID cb_id = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(timer_inuse());

  StatusCode ret;
  ret = timer_start_ms(1, prv_timeout_cb, &cb_id, &id);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id);

  uint32_t prev_time_remaining = timer_remaining_time(id);
  while (cb_id == SOFT_TIMER_INVALID_TIMER) {
    uint32_t time_remaining = timer_remaining_time(id);
    TEST_ASSERT_TRUE(time_remaining <= prev_time_remaining);
    prev_time_remaining = time_remaining;
  }

  TEST_ASSERT_EQUAL(0, timer_remaining_time(id));
  TEST_ASSERT_EQUAL(id, cb_id);

  TEST_ASSERT_FALSE(timer_inuse());
}
