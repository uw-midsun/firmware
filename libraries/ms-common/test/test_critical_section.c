#include "critical_section.h"

#include <stdbool.h>

#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {
  // After a test concludes forcibly enable interrupts if they aren't already
  // enabled.
  critical_section_end(true);
}

// Verifies the logic of critical sections. The validation of internal behavior
// needs to be
// performed at an interrupt module level ie in test_gpio_it.c.
void test_interrupt_disable_enable(void) {
  bool disabled = critical_section_start();
  TEST_ASSERT_TRUE(disabled);
  bool also_disabled = critical_section_start();
  TEST_ASSERT_FALSE(also_disabled);
  critical_section_end(also_disabled);
  TEST_ASSERT_FALSE(critical_section_start());
  critical_section_end(disabled);
  TEST_ASSERT_TRUE(critical_section_start());
}
