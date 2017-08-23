#include "pwm.h"

#include <stdint.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define PERIOD_MS 100
#define WAIT_CYCLES 5

static volatile uint16_t s_counter = 0;

static void prv_pwm_test(const GPIOAddress *address, void *context) {
  s_counter++;
}

void setup_test(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  s_counter = 0;
}

void teardown_test(void) {}

void test_pwm_guards(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, pwm_set_pulse(PWM_TIMER_3, 50));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(PWM_TIMER_3, 0));
  TEST_ASSERT_OK(pwm_init(PWM_TIMER_3, PERIOD_MS));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_dc(PWM_TIMER_3, 101));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_set_pulse(PWM_TIMER_3, PERIOD_MS + 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, pwm_init(NUM_PWM_TIMERS, PERIOD_MS));
}

void test_pwm_output(void) {
  TEST_ASSERT_OK(pwm_init(PWM_TIMER_3, PERIOD_MS));
  TEST_ASSERT_EQUAL(PERIOD_MS, pwm_get_period(PWM_TIMER_3));
  TEST_ASSERT_OK(pwm_set_pulse(PWM_TIMER_3, PERIOD_MS - 1));
  TEST_ASSERT_OK(pwm_set_dc(PWM_TIMER_3, 50));

  const GPIOAddress addr = {
    .port = GPIO_PORT_A,  //
    .pin = 6,             //
  };
  const GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_1,
  };
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };
  TEST_ASSERT_OK(gpio_init_pin(&addr, &settings));
  TEST_ASSERT_OK(gpio_it_register_interrupt(&addr, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                                            prv_pwm_test, NULL));
  while (s_counter < WAIT_CYCLES) {
  }

  TEST_ASSERT_TRUE(s_counter >= WAIT_CYCLES);
}
