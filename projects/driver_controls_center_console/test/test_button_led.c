#include "button_led.h"

#include <inttypes.h>
#include <stddef.h>

#include "unity.h"

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

// Iterate through all the states of each relay to validate their behavior with retry logic based on
// the 4 different success/failure combinations for opening and closing. Essentially checks all
// success cases, handling of failures, etc.
void test_button_led_cycle(void) {
  //
}
