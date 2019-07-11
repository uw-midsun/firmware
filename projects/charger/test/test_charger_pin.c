#include "charger_pin.h"

#include "adc.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  gpio_it_init();
  event_queue_init();
  adc_init(ADC_MODE_CONTINUOUS);
}

void teardown_test(void) {}

void test_charger_pin(void) {
  const GpioAddress addr = {
    .pin = 1,
    .port = 1,
  };
  TEST_ASSERT_OK(charger_pin_init(&addr));
  delay_ms(CHARGER_PIN_POLL_PERIOD_MS + 100);

  // Initial signal. Will be low if no hardware is connected.
  Event e = { 0, 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHARGER_EVENT_DISCONNECTED, e.id);
}
