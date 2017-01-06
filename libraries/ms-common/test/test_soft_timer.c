#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "extra_unity.h"
#include "unity.h"

void setup_test(void) {
  // This will be 4x slow on the stm32f0 discovery as it only has an 8 MHz ext clock. The controller
  // board is 32 MHz.
  soft_timer_init(32);
}

void teardown_test(void) { }

static bool s_callback_ran = false;
static uint8_t s_interrupt_id = 255;

void prv_soft_timer_test_callback(uint8_t id, void* context) {
  s_interrupt_id = id;
  s_callback_ran = true;
}

// Software timer end to end test.
void test_soft_timer_end2end(void) {
  uint8_t timer_id = 255;
  // 1 second timer.
  TEST_ASSERT_OK(soft_timer_start(1000000, prv_soft_timer_test_callback, &timer_id, NULL));
  // Assume the timer id changed to something else. (Not a specific number as this would prevent the
  // test from passing if we ever modified the internals).
  TEST_ASSERT_NOT_EQUAL(255, timer_id);
  TEST_ASSERT_FALSE(s_callback_ran);

  // Update assuming the above operation is faster than the timer.
  soft_timer_update();
  TEST_ASSERT_FALSE(s_callback_ran);

  // Until the callback gets run wait.
  while (!s_callback_ran) {
    soft_timer_update();
  }
  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_EQUAL(timer_id, s_interrupt_id);
}

// Software timer create too many
void test_soft_timer_resource_exhausted(void) {
  uint8_t timer_id = 255;
  // Empty timer.
  for (int i = 0; i < SOFT_TIMER_NUM; i++) {
    TEST_ASSERT_OK(soft_timer_start(0, prv_soft_timer_test_callback, &timer_id, NULL));
  }

  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED,
    soft_timer_start(0, prv_soft_timer_test_callback, &timer_id, NULL));
}
