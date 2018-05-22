#include "critical_section.h"

#include <stdbool.h>

#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {
  // After a test concludes forcibly enable interrupts if they aren't already enabled.
  CriticalSection section = { .disabled_in_scope = true };
  critical_section_end(&section);
}

// Verifies the logic of critical sections. The validation of internal behavior needs to be
// performed at an interrupt module level ie in test_gpio_it.c.
void test_critical_section_interrupt_disable_enable(void) {
  CriticalSection disabled = critical_section_start();
  TEST_ASSERT_TRUE(disabled.disabled_in_scope);
  CriticalSection also_disabled = critical_section_start();
  TEST_ASSERT_FALSE(also_disabled.disabled_in_scope);
  critical_section_end(&also_disabled);

  CriticalSection section = critical_section_start();
  TEST_ASSERT_FALSE(section.disabled_in_scope);
  critical_section_end(&disabled);
  section = critical_section_start();
  TEST_ASSERT_TRUE(section.disabled_in_scope);
  critical_section_end(&section);
}

static void prv_section_cleanup(void) {
  CRITICAL_SECTION_AUTOEND;
  TEST_ASSERT_TRUE(_section.disabled_in_scope);
}

void test_critical_section_cleanup(void) {
  prv_section_cleanup();

  // Previous critical section should be cleaned up automatically
  CriticalSection section = critical_section_start();
  TEST_ASSERT_TRUE(section.disabled_in_scope);
  critical_section_end(&section);
}
