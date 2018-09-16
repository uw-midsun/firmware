#include "charger_pin.h"

#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_charger_pin(void) {
  const GpioAddress addr = {
    .pin = 1,
    .port = 1,
  };
  TEST_ASSERT_OK(charger_pin_init(&addr));

  // Initial signal as to which state to start in (should be low).
  Event e = { 0, 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHARGER_EVENT_DISCONNECTED, e.id);

  // Validate the interrupt causes the same behavior.
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&addr));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHARGER_EVENT_DISCONNECTED, e.id);
}
