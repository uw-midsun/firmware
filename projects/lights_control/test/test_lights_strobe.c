#include "test_helpers.h"
#include "unity.h"
#include "status.h"

#include "lights_blinker.h"
#include "lights_events.h"
#include "lights_strobe.h"

#define TEST_LIGHTS_STROBE_DURATION 300

static LightsStrobe s_lights_strobe = { 0 };

void setup_test(void) {
  event_queue_init();
  lights_strobe_init(&s_lights_strobe, TEST_LIGHTS_STROBE_DURATION);
}

static void prv_assert_raised_event(LightsEvent event) {
  const Event raised_event = { 0 };
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(raised_event.id, event);
}

static void prv_assert_no_event_raised() {
  // Clearing event.
  const Event raised_event = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&raised_event));
}

void test_lights_strobe_turn_on_off() {
  // Turning strobe on.
  const Event e1 = {
    .id = LIGHTS_EVENT_STROBE_ON,      //
    .data = 0  //
  };
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e1));
  const Event raised_event = { 0 };
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(raised_event.id, LIGHTS_EVENT_GPIO_ON);
  TEST_ASSERT_EQUAL(raised_event.data, LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE);

  // Waiting for one blink.
  while (!status_ok(event_process(&raised_event))) {}
  // Asserting correct event raised by blinker.
  TEST_ASSERT_EQUAL(raised_event.id, LIGHTS_EVENT_GPIO_OFF);
  TEST_ASSERT_EQUAL(raised_event.data, LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE);

  // Turning strobe off.
  const Event e2 = {
    .id = LIGHTS_EVENT_STROBE_OFF,       //
    .data = 0  //
  };
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e2));
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(raised_event.id, LIGHTS_EVENT_GPIO_OFF);
  TEST_ASSERT_EQUAL(raised_event.data, LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE);
}

// Do nothing when event is not LIGHTS_EVENT_STROBE
void test_lights_strobe_non_strobe_event() {
  // Left signal event.
  const Event e1 = {
    .id = LIGHTS_EVENT_SIGNAL_ON,  //
    .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT   //
  };
  // Do nothing.
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e1));
  TEST_ASSERT_EQUAL(
  prv_assert_no_event_raised();
}

void teardown_test() {}
