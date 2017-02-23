#include "soft_timer.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include "interrupt.h"
#include "test_helpers.h"
#include "unity.h"

// Medium term callback used across tests.
static volatile bool s_callback_ran = false;
static volatile uint8_t s_interrupt_id = 255;

static void prv_soft_timer_test_callback(SoftTimerID id, void* context) {
  s_interrupt_id = id;
  s_callback_ran = true;
}

// Short duration callback used in multi timer test.
static volatile bool s_short_callback_ran = false;
static volatile uint8_t s_short_interrupt_id = 255;

static void prv_short_soft_timer_test_callback(SoftTimerID id, void* context) {
  s_short_interrupt_id = id;
  s_short_callback_ran = true;
}

// Long duration callback used in the multi timer test.
static volatile bool s_long_callback_ran = false;
static volatile uint8_t s_long_interrupt_id = 255;

static void prv_long_soft_timer_test_callback(SoftTimerID id, void* context) {
  s_long_interrupt_id = id;
  s_long_callback_ran = true;
}

void setup_test(void) {
  // Reset all timers/interrupts and reset the callback used in all tests.
  interrupt_init();
  soft_timer_init();
  s_short_callback_ran = false;
  s_callback_ran = false;
  s_long_callback_ran = false;
}

void teardown_test(void) {}

// Software timer end to end test.
void test_soft_timer_end2end(void) {
  SoftTimerID timer_id = 255;

  TEST_ASSERT_FALSE(soft_timer_inuse());

  // 0.5 second timer.
  TEST_ASSERT_OK(soft_timer_start(500, prv_soft_timer_test_callback, NULL, &timer_id));

  // Assume the timer id changed to something else. (Not a specific number as this would prevent the
  // test from passing if we ever modified the internals).
  TEST_ASSERT_NOT_EQUAL(255, timer_id);

  // Validate it hasn't run yet (possibly flaky);
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_TRUE(soft_timer_inuse());

  while (!s_callback_ran) {
  }

  // Once run validate everything is as expected.
  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_EQUAL(timer_id, s_interrupt_id);
}

void test_soft_timer_multitimer(void) {
  SoftTimerID short_timer = 255;
  SoftTimerID mid_timer = 255;
  SoftTimerID long_timer = 255;

  // Verify everything is clear.
  TEST_ASSERT_FALSE(soft_timer_inuse());

  // 1 Second timer starting immediately
  TEST_ASSERT_OK(soft_timer_start_millis(1, prv_soft_timer_test_callback, NULL, &mid_timer));
  // 0.5 second timer premepts the other timer.
  TEST_ASSERT_OK(soft_timer_start(500, prv_short_soft_timer_test_callback, NULL, &short_timer));
  // Queued last timer should return after 2 seconds.
  TEST_ASSERT_OK(soft_timer_start_seconds(1, prv_long_soft_timer_test_callback, NULL, &long_timer));

  while (!s_short_callback_ran) {
  }

  // Once the short timer ran make sure the medium timer hasn't run (can be flaky for very short
  // durations).
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_EQUAL(short_timer, s_short_interrupt_id);

  while (!s_callback_ran) {
  }

  // Once the mid timer ran make sure the longer timer hasn't run (can be flaky for very short
  // durations).
  TEST_ASSERT_FALSE(s_long_callback_ran);
  TEST_ASSERT_EQUAL(mid_timer, s_interrupt_id);

  while (!s_long_callback_ran) {
  }

  // Verify the long timer ran.
  TEST_ASSERT_EQUAL(long_timer, s_long_interrupt_id);
}

void test_soft_timer_multitimer_short(void) {
  SoftTimerID short_timer = 255;
  SoftTimerID mid_timer = 255;
  SoftTimerID long_timer = 255;

  // Verify everything is clear.
  TEST_ASSERT_FALSE(soft_timer_inuse());

  // 1 Second timer starting immediately
  TEST_ASSERT_OK(soft_timer_start(125, prv_soft_timer_test_callback, NULL, &mid_timer));
  // 0.5 second timer premepts the other timer.
  TEST_ASSERT_OK(soft_timer_start(50, prv_short_soft_timer_test_callback, NULL, &short_timer));
  // Queued last timer should return after 2 seconds.
  TEST_ASSERT_OK(soft_timer_start(200, prv_long_soft_timer_test_callback, NULL, &long_timer));

  while (!s_short_callback_ran) {
  }

  // Once the short timer ran make sure the medium timer hasn't run (can be flaky for very short
  // durations).
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_EQUAL(short_timer, s_short_interrupt_id);

  while (!s_callback_ran) {
  }

  // Once the mid timer ran make sure the longer timer hasn't run (can be flaky for very short
  // durations).
  TEST_ASSERT_FALSE(s_long_callback_ran);
  TEST_ASSERT_EQUAL(mid_timer, s_interrupt_id);

  while (!s_long_callback_ran) {
  }

  // Verify the long timer ran.
  TEST_ASSERT_EQUAL(long_timer, s_long_interrupt_id);
}

// Test for rollover.
void test_soft_timer_rollover(void) {
  SoftTimerID timer_id = 255;

  TEST_ASSERT_FALSE(soft_timer_inuse());
  TEST_soft_timer_set_counter(UINT32_MAX - 500);
  TEST_ASSERT_OK(soft_timer_start(1000, prv_soft_timer_test_callback, NULL, &timer_id));
  TEST_ASSERT_NOT_EQUAL(255, timer_id);

  // Validate it hasn't run yet (possibly flaky).
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_TRUE(soft_timer_inuse());

  while (!s_callback_ran) {
  }

  // Once run, validated everything is as expected.
  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_EQUAL(timer_id, s_interrupt_id);
}

// Test cancellation.
void test_soft_timer_cancel(void) {
  SoftTimerID timer_id = 255;

  TEST_ASSERT_FALSE(soft_timer_inuse());
  TEST_ASSERT_FALSE(soft_timer_cancel(timer_id));
  TEST_ASSERT_OK(soft_timer_start(1000, prv_long_soft_timer_test_callback, NULL, &timer_id));
  TEST_ASSERT_OK(soft_timer_start(500, prv_soft_timer_test_callback, NULL, &timer_id));
  TEST_ASSERT_NOT_EQUAL(255, timer_id);

  // Validate this hasn't run yet (possibly flaky).
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_TRUE(soft_timer_inuse());

  // Check we can cancel.
  TEST_ASSERT_TRUE(soft_timer_cancel(timer_id));

  while (!s_long_callback_ran) {
  }

  // Validate it still hasn't run.
  TEST_ASSERT_FALSE(s_callback_ran);
}

// Create too many software timers.
void test_soft_timer_resource_exhausted(void) {
  SoftTimerID timer_id = 255;

  // Start a bunch of 10 second timers we will cancel them before they run.
  for (int i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    TEST_ASSERT_OK(soft_timer_start(10000000, prv_soft_timer_test_callback, NULL, &timer_id));
  }

  // Verify we run out of timers.
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED,
                    soft_timer_start(10000000, prv_soft_timer_test_callback, NULL, &timer_id));

  // Reset all timers before exiting to prevent any strange behaviors.
  soft_timer_init();
}
