#include "event_queue.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_events.h"
#include "lights_simple_peripherals.h"

#define TEST_LIGHTS_SIMPLE_STATE_ON 1
#define TEST_LIGHTS_SIMPLE_STATE_OFF 0

static StatusCode prv_custom_callback(const Event *e) {
  // these events should not be processed, thus callback shouldn't be called with them
  switch (e->id) {
    case LIGHTS_EVENT_SIGNAL_LEFT:
    case LIGHTS_EVENT_SIGNAL_RIGHT:
    case LIGHTS_EVENT_SIGNAL_HAZARD:
    case LIGHTS_EVENT_SYNC:
    case LIGHTS_EVENT_STROBE:
      TEST_FAIL();
  }
  uint16_t callback_data_lookup[] = {
    [LIGHTS_EVENT_HORN] = TEST_LIGHTS_SIMPLE_STATE_ON,        //
    [LIGHTS_EVENT_HIGH_BEAMS] = TEST_LIGHTS_SIMPLE_STATE_ON,  //
    [LIGHTS_EVENT_LOW_BEAMS] = TEST_LIGHTS_SIMPLE_STATE_OFF,  //
    [LIGHTS_EVENT_DRL] = TEST_LIGHTS_SIMPLE_STATE_OFF,        //
    [LIGHTS_EVENT_BRAKES] = TEST_LIGHTS_SIMPLE_STATE_ON,      //
  };
  TEST_ASSERT_EQUAL(e->data, callback_data_lookup[e->id]);
  return STATUS_CODE_OK;
}

// registers the custom callback we provided
void setup_test(void) {
  TEST_ASSERT_OK(lights_simple_peripherals_init(prv_custom_callback));
}

void teardown_test(void) {}

// we pass in events, and see the corresponding action taken by this module
void test_lights_simple_peripherals_event(void) {
  uint16_t event_data_lookup[] = {
    [LIGHTS_EVENT_HORN] = TEST_LIGHTS_SIMPLE_STATE_ON,           //
    [LIGHTS_EVENT_HIGH_BEAMS] = TEST_LIGHTS_SIMPLE_STATE_ON,     //
    [LIGHTS_EVENT_LOW_BEAMS] = TEST_LIGHTS_SIMPLE_STATE_OFF,     //
    [LIGHTS_EVENT_DRL] = TEST_LIGHTS_SIMPLE_STATE_OFF,           //
    [LIGHTS_EVENT_BRAKES] = TEST_LIGHTS_SIMPLE_STATE_ON,         //
    [LIGHTS_EVENT_SIGNAL_RIGHT] = TEST_LIGHTS_SIMPLE_STATE_ON,   //
    [LIGHTS_EVENT_SIGNAL_LEFT] = TEST_LIGHTS_SIMPLE_STATE_OFF,   //
    [LIGHTS_EVENT_SIGNAL_HAZARD] = TEST_LIGHTS_SIMPLE_STATE_ON,  //
    [LIGHTS_EVENT_SYNC] = TEST_LIGHTS_SIMPLE_STATE_OFF,          //
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(event_data_lookup); i++) {
    const Event e = { .id = i, .data = event_data_lookup[i] };
    TEST_ASSERT_OK(lights_simple_peripherals_process_event(&e));
  }
}
