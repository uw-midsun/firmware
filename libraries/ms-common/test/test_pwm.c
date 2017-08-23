#include "pwm.h"

#include "test_helpers.h"
#include "unity.h"

// TODO(ELEC-263): Consider adding functional or more rigorous testing.

#define PERIOD_MS 100

void setup_test(void) {}

void teardown_test(void) {}

void test_pwm_guards(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, pwm_set_pulse(PWM_TIMER_3, 50));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(PWM_TIMER_3, 0));
  TEST_ASSERT_OK(pwm_init(PWM_TIMER_3, PERIOD_MS));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_dc(PWM_TIMER_3, 101));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_pulse(PWM_TIMER_3, PERIOD_MS + 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(NUM_PWM_TIMERS, PERIOD_MS));
}

void test_pwm_valid_arg(void) {
  TEST_ASSERT_OK(pwm_init(PWM_TIMER_3, PERIOD_MS));
  TEST_ASSERT_EQUAL(PERIOD_MS, pwm_get_period(PWM_TIMER_3));
  TEST_ASSERT_OK(pwm_set_dc(PWM_TIMER_3, 50));
  TEST_ASSERT_OK(pwm_set_pulse(PWM_TIMER_3, PERIOD_MS - 1));
}
