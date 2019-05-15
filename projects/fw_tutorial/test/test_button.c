#include "test_helpers.h"
#include "button.h"
#include "config.h"

static ButtonStorage button_storage[NUM_BUTTON_COLOURS];

StatusCode TEST_MOCK(gpio_it_register_interrupt) (const GpioAddress *address,
                                                  const InterruptSettings *settings,
                                                  InterruptEdge edge,
                                                  GpioItCallback callback,
                                                  void *context) {
  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

void test_invalid_input(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, button_init(NULL, button_storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, button_init(config_load_buttons(), NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, button_init(NULL, NULL));
}

void test_valid_input(void) {
  TEST_ASSERT_OK(button_init(config_load_buttons(), button_storage));
}
