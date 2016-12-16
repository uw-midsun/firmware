#include "interrupt.h"

#include <stdbool.h>

#include "extra_unity.h"
#include "gpio.h"
#include "gpio_cfg.h"
#include "status.h"
#include "unity.h"

#define VALID_PORT 0
#define VALID_PIN 0
#define INVALID_PORT NUM_GPIO_PINS
#define INVALID_PIN NUM_GPIO_PORTS

static bool s_callback_ran;

static GPIOAddress s_valid_address = { VALID_PORT, VALID_PIN };

static InterruptSettings s_default_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_EDGE_RISING,
                                                INTERRUPT_PRIORITY_NORMAL };

static void prv_test_callback() {
  s_callback_ran = true;
}

void setup_test(void) {
  interrupt_init();
  s_callback_ran = false;
}

void teardown_test(void) {}

// Verifies that gaurd clauses all work as intended for GPIO.
void test_interrupt_gpio_invalid_cases(void) {
  GPIOAddress invalid_address = { INVALID_PORT, INVALID_PIN };
  InterruptSettings invalid_settings = { NUM_INTERRUPT_TYPE, INTERRUPT_EDGE_RISING,
                                         INTERRUPT_PRIORITY_NORMAL };
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      interrupt_gpio_register(&invalid_address, &s_default_settings, prv_test_callback));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, interrupt_gpio_trigger(&invalid_address));
  invalid_address.port = VALID_PORT;
  invalid_address.port = INVALID_PIN;
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      interrupt_gpio_register(&invalid_address, &s_default_settings, prv_test_callback));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, interrupt_gpio_trigger(&invalid_address));

  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      interrupt_gpio_register(&s_valid_address, &invalid_settings, prv_test_callback));
  invalid_settings.type = INTERRUPT_TYPE_INTERRUPT;
  invalid_settings.edge = NUM_INTERRUPT_EDGE;
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      interrupt_gpio_register(&s_valid_address, &invalid_settings, prv_test_callback));
  invalid_settings.edge = INTERRUPT_EDGE_RISING;
  invalid_settings.priority = NUM_INTERRUPT_PRIORITY;

  TEST_ASSERT_OK(interrupt_gpio_register(&s_valid_address, &s_default_settings, prv_test_callback));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_RESOURCE_EXHAUSTED,
      interrupt_gpio_register(&s_valid_address, &s_default_settings, prv_test_callback));
}

// Verifies that software interrupts work for GPIO.
void test_interrupt_gpio_software_trigger(void) {
  TEST_ASSERT_OK(interrupt_gpio_trigger(&s_valid_address));
  TEST_ASSERT_TRUE(s_callback_ran);
}

// Verifies that critical sections both nexted and un-nested work
void test_interrupt_critical_sections(void) {
  BEGIN_CRITICAL_SECTION();
  TEST_ASSERT_OK(interrupt_gpio_trigger(&s_valid_address));
  TEST_ASSERT_FALSE(s_callback_ran);
  BEGIN_CRITICAL_SECTION();
  TEST_ASSERT_FALSE(s_callback_ran);
  END_CRITICAL_SECTION();
  TEST_ASSERT_FALSE(s_callback_ran);
  END_CRITICAL_SECTION();
  TEST_ASSERT_TRUE(s_callback_ran);
}
