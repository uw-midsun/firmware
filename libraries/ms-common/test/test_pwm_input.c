#include "pwm_input.h"

#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "pwm.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// To run this test, connect the input and output pins

#define TEST_OUTPUT_PWM_TIMER PWM_TIMER_1
#define TEST_OUTPUT_PWM_ALTFN GPIO_ALTFN_2
#define TEST_OUTPUT_PWM_PERIOD_US 1000
#define TEST_OUTPUT_PWM_ADDR \
  { .port = GPIO_PORT_A, .pin = 8, }

#define TEST_INPUT_PWM_TIMER PWM_TIMER_3
#define TEST_INPUT_PWM_ALTFN GPIO_ALTFN_1
#define TEST_INPUT_PWM_CHANNEL PWM_CHANNEL_2
#define TEST_INPUT_PWM_ADDR \
  { .port = GPIO_PORT_B, .pin = 5, }

#define TOLERANCE (2)

volatile uint32_t dc = 0;
volatile uint32_t period = 0;

PwmInputStorage storage = { 0 };

static void prv_dc_callback(uint32_t local_dc, uint32_t local_period, void *context) {
  dc = local_dc;
  period = local_period;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
}

void teardown_test(void) {}

void test_pwm_input(void) {
  PwmTimer input_timer = TEST_INPUT_PWM_TIMER;
  PwmTimer output_timer = TEST_OUTPUT_PWM_TIMER;

  GpioAddress input = TEST_INPUT_PWM_ADDR;

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = TEST_INPUT_PWM_ALTFN,
  };

  GpioAddress output = TEST_OUTPUT_PWM_ADDR;

  GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = TEST_OUTPUT_PWM_ALTFN,
  };

  PwmInputSettings pwm_input_settings = {
    .channel = TEST_INPUT_PWM_CHANNEL,
    .callback = prv_dc_callback,
  };

  TEST_ASSERT_OK(gpio_init_pin(&output, &output_settings));
  TEST_ASSERT_OK(gpio_init_pin(&input, &input_settings));

  TEST_ASSERT_OK(pwm_init(TEST_OUTPUT_PWM_TIMER, TEST_OUTPUT_PWM_PERIOD_US));

  TEST_ASSERT_OK(pwm_input_init(input_timer, &pwm_input_settings, &storage));

  for (int i = 0; i <= 100; i++) {
    TEST_ASSERT_OK(pwm_set_dc(TEST_OUTPUT_PWM_TIMER, i));
    delay_ms(50);
    uint32_t get_dc = pwm_input_get_dc(TEST_INPUT_PWM_TIMER);
    LOG_DEBUG("DC: %d, read DC: %d\n", i, (int)get_dc);
    TEST_ASSERT_TRUE((uint32_t)i * 10 + TOLERANCE > get_dc);
    TEST_ASSERT_TRUE(((uint32_t)i == 0 ? (uint32_t)0 : (uint32_t)i * 10 - TOLERANCE) <= get_dc);
  }
}
