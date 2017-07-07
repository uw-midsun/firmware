#include "pwm.h"

#include "test_helpers.h"
#include "unity.h"

#define PERIOD_MS 100

void setup_test(void) { }

void teardown_test(void) { }

void test_pwm_guards(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, pwm_set_pulse(50));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(0));
  TEST_ASSERT_OK(pwm_init(PERIOD_MS));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_dc(101));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_pulse(PERIOD_MS + 1));
}

void test_pwm_valid_arg(void) {
  TEST_ASSERT_OK(pwm_init(PERIOD_MS));
  TEST_ASSERT_EQUAL(PERIOD_MS, pwm_get_period());
  TEST_ASSERT_OK(pwm_set_dc(50));
  TEST_ASSERT_OK(pwm_set_pulse(PERIOD_MS - 1));
}
