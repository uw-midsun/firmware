#include "button.h"
#include "config.h"
#include "test_helpers.h"

// Button storage
static ButtonStorage button_storage[NUM_BUTTON_COLOURS];

// Mocked `gpio_it_register_interrupt` function
StatusCode TEST_MOCK(gpio_it_register_interrupt)(const GpioAddress *address,
                                                 const InterruptSettings *settings,
                                                 InterruptEdge edge, GpioItCallback callback,
                                                 void *context) {
  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

void test_invalid_input(void) {
  // Test for invalid inputs
  // Enter your code below ...
}

void test_valid_input(void) {
  // Test for valid inputs
  // Enter your code below ...
}
