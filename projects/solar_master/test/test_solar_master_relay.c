#include <stdint.h>

#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "misc.h"
#include "test_helpers.h"
#include "unity.h"

#include "solar_master_event.h"
#include "solar_master_relay.h"

static const GpioAddress s_relay_address = {
  .port = GPIO_PORT_A,  //
  .pin = 8,             //
};

void setup_test(void) {
  TEST_ASSERT_OK(gpio_init());
  TEST_ASSERT_OK(solar_master_relay_init());
}

void teardown_test(void) {}

void test_solar_master_relay_init(void) {
  GpioState state;
  TEST_ASSERT_OK(gpio_get_state(&s_relay_address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}

void test_solar_master_relay_process_event(void) {
  Event test_event = { .id = SOLAR_MASTER_EVENT_RELAY_STATE, .data = EE_RELAY_STATE_CLOSE };
  GpioState state;
  TEST_ASSERT_OK(solar_master_relay_process_event(&test_event));
  TEST_ASSERT_OK(gpio_get_state(&s_relay_address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

  test_event.data = EE_CHARGER_SET_RELAY_STATE_OPEN;
  TEST_ASSERT_OK(solar_master_relay_process_event(&test_event));
  TEST_ASSERT_OK(gpio_get_state(&s_relay_address, &state));
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}
