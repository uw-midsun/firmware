#include "gpio_seq.h"

#include <stdint.h>

#include "gpio.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_GPIO_SEQ_DELAY_US 300

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_gpio_seq(void) {
  gpio_seq_set_delay(TEST_GPIO_SEQ_DELAY_US);
  TEST_ASSERT_EQUAL_UINT32(TEST_GPIO_SEQ_DELAY_US, gpio_seq_get_delay());

  const GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  const GPIOAddress addrs[] = {
    { .port = 0, .pin = 1 },
    { .port = 0, .pin = 2 },
    { .port = 0, .pin = 3 },
  };

  TEST_ASSERT_OK(gpio_seq_init_pins(addrs, SIZEOF_ARRAY(addrs), &settings));
  TEST_ASSERT_OK(gpio_seq_set_state(addrs, SIZEOF_ARRAY(addrs), GPIO_STATE_HIGH));

  GPIOState state = GPIO_STATE_LOW;
  for (uint16_t i = 0; i < SIZEOF_ARRAY(addrs); i++) {
    TEST_ASSERT_OK(gpio_get_state(&addrs[i], &state));
    TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
  }
}
