#include "event_queue.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "simple_peripherals.h"
#include "lights_events.h"

static StatusCode prv_custom_callback(Event e) {
  switch (e.id) {
    case EVENT_SIGNAL_LEFT:
    case EVENT_SIGNAL_RIGHT:
    case EVENT_SIGNAL_HAZARD:
    case EVENT_SYNC:
    case EVENT_STROBE:
      TEST_FAIL();
  }
  uint16_t callback_data_lookup[] = {
    [EVENT_HORN] = 5, //
    [EVENT_HEADLIGHTS] = 3, //
    [EVENT_BRAKES] = 2 //
  };
  TEST_ASSERT_EQUAL(e.data, callback_data_lookup[e.id]);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  TEST_ASSERT_OK(simple_peripherals_init(prv_custom_callback));
}

void teardown_test(void) {}

void test_simple_peripherals_event(void) {
  uint16_t event_data_lookup[] = {
    [EVENT_HORN] = 5, //
    [EVENT_HEADLIGHTS] = 3, //
    [EVENT_BRAKES] = 2, //
    [EVENT_STROBE] = 7, //
    [EVENT_SIGNAL_LEFT] = 2, //
    [EVENT_SIGNAL_RIGHT] = 4, //
    [EVENT_SIGNAL_HAZARD] = 9, //
    [EVENT_SYNC] = 13 //
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(event_data_lookup); i++) {
    Event e = {
      .id = i,
      .data = event_data_lookup[i]
    };
    TEST_ASSERT_OK(simple_peripherals_event(e));
  }
}

