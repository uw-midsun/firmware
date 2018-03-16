#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "critical_section.h"
#include "hal_test_helpers.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct TestSoftTimerRepetitiveCtx {
  uint32_t period;
  uint16_t counter;
  SoftTimerID id;
} TestSoftTimerRepetitiveCtx;

static void prv_repetitive_timer(SoftTimerID timer_id, void *context) {
  (void)timer_id;
  TestSoftTimerRepetitiveCtx *rctx = context;
  rctx->counter++;
  soft_timer_start(rctx->period, prv_repetitive_timer, context, &rctx->id);
}

static void prv_timeout_cb(SoftTimerID timer_id, void *context) {
  SoftTimerID *cb_id = context;
  *cb_id = timer_id;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();

  // Purposely start close to the rollover
  _test_soft_timer_set_counter(UINT32_MAX - 2000);
}

void teardown_test(void) {}

void test_soft_timer_basic(void) {
  volatile SoftTimerID cb_id = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(soft_timer_inuse());

  StatusCode ret;
  ret = soft_timer_start_millis(1, prv_timeout_cb, (void *)&cb_id, &id);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id);
  TEST_ASSERT_TRUE(soft_timer_inuse());

  while (cb_id == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id, cb_id);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_soft_timer_preempt(void) {
  // Begin medium, longer, short, then long - should finish short, medium, long, longer
  // Tests ordering: new, append to back, insert into front, insert into middle
  volatile SoftTimerID cb_id_short = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_medium = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_long = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_longer = SOFT_TIMER_INVALID_TIMER;

  SoftTimerID id_short = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_medium = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_long = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_longer = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(soft_timer_inuse());

  StatusCode ret;

  ret = soft_timer_start_millis(1, prv_timeout_cb, (void *)&cb_id_medium, &id_medium);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_medium);

  ret = soft_timer_start_seconds(1, prv_timeout_cb, (void *)&cb_id_longer, &id_longer);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_longer);

  ret = soft_timer_start(500, prv_timeout_cb, (void *)&cb_id_short, &id_short);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_short);

  ret = soft_timer_start_millis(500, prv_timeout_cb, (void *)&cb_id_long, &id_long);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_long);

  TEST_ASSERT_TRUE(soft_timer_inuse());

  while (cb_id_short == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_short, cb_id_short);
  TEST_ASSERT_NOT_EQUAL(id_medium, cb_id_medium);
  TEST_ASSERT_NOT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_medium == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_medium, cb_id_medium);
  TEST_ASSERT_NOT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_long == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_longer, cb_id_longer);

  while (cb_id_longer == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_longer, cb_id_longer);

  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_soft_timer_cancelled_timer(void) {
  volatile SoftTimerID cb_id_short = SOFT_TIMER_INVALID_TIMER;
  volatile SoftTimerID cb_id_long = SOFT_TIMER_INVALID_TIMER;

  SoftTimerID id_short = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id_long = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(soft_timer_inuse());

  StatusCode ret;
  ret = soft_timer_start_millis(1, prv_timeout_cb, (void *)&cb_id_long, &id_long);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_long);

  ret = soft_timer_start(10, prv_timeout_cb, (void *)&cb_id_short, &id_short);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id_short);

  soft_timer_cancel(id_short);

  while (cb_id_long == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_long, cb_id_long);
  TEST_ASSERT_NOT_EQUAL(id_short, cb_id_short);

  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_soft_timer_remaining(void) {
  volatile SoftTimerID cb_id = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID id = SOFT_TIMER_INVALID_TIMER;

  TEST_ASSERT_FALSE(soft_timer_inuse());

  StatusCode ret;
  ret = soft_timer_start_millis(1, prv_timeout_cb, (void *)&cb_id, &id);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, id);

  uint32_t prev_time_remaining = soft_timer_remaining_time(id);
  while (cb_id == SOFT_TIMER_INVALID_TIMER) {
    bool crit = critical_section_start();
    uint32_t time_remaining = soft_timer_remaining_time(id);
    TEST_ASSERT_TRUE(time_remaining <= prev_time_remaining);
    critical_section_end(crit);
    prev_time_remaining = time_remaining;
  }

  TEST_ASSERT_EQUAL(0, soft_timer_remaining_time(id));
  TEST_ASSERT_EQUAL(id, cb_id);

  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_soft_timer_exhausted(void) {
  volatile SoftTimerID cb_ids[SOFT_TIMER_MAX_TIMERS] = { 0 };
  volatile SoftTimerID cb_id_single = SOFT_TIMER_INVALID_TIMER;
  SoftTimerID ids[SOFT_TIMER_MAX_TIMERS] = { 0 };
  SoftTimerID id_single = SOFT_TIMER_INVALID_TIMER;

  StatusCode ret;
  for (int i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    cb_ids[i] = SOFT_TIMER_INVALID_TIMER;
    ret = soft_timer_start_millis(10, prv_timeout_cb, (void *)&cb_ids[i], &ids[i]);
    TEST_ASSERT_OK(ret);
  }

  ret = soft_timer_start(1, prv_timeout_cb, (void *)&cb_id_single, NULL);
  TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, ret);

  // Cancel arbitrary timer
  soft_timer_cancel(ids[3]);

  ret = soft_timer_start_millis(1, prv_timeout_cb, (void *)&cb_id_single, &id_single);
  TEST_ASSERT_OK(ret);

  while (cb_id_single == SOFT_TIMER_INVALID_TIMER) {
  }

  TEST_ASSERT_EQUAL(id_single, cb_id_single);
  TEST_ASSERT_NOT_EQUAL(ids[0], cb_ids[0]);

  TEST_ASSERT_TRUE(soft_timer_inuse());

  for (int i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    soft_timer_cancel(ids[i]);
  }

  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_fast_timer(void) {
  volatile TestSoftTimerRepetitiveCtx rctx = {
    .counter = 0,
    .period = 10,  // us
    .id = SOFT_TIMER_INVALID_TIMER,
  };
  TEST_ASSERT_OK(soft_timer_start(rctx.period, prv_repetitive_timer, (void *)&rctx, &rctx.id));
  while (rctx.counter < 500) {
  }
  soft_timer_cancel(rctx.id);
}
