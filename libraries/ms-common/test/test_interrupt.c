#include "interrupt.h"

#include <stdbool.h>

#include "unity.h"

void setup_test(void) { }

void teardown_test(void) {
  // After a test concludes forcibly enable interrupts if they aren't already enabled.
  interrupt_enable(false);
}

// Verifies the logic of interrupt disable and enable. The validation of internal behavior needs to
// be performed at an interrupt module level ie in test_gpio_it.c.
void test_interrupt_disable_enable(void) {
  bool was_disabled = interrupt_disable();
  TEST_ASSERT_FALSE(was_disabled);
  bool was_also_disabled = interrupt_disable();
  TEST_ASSERT_TRUE(was_also_disabled);
  interrupt_enable(was_also_disabled);
  TEST_ASSERT_FALSE(interrupt_disable());
  interrupt_enable(was_disabled);
  TEST_ASSERT_TRUE(interrupt_disable());
}
