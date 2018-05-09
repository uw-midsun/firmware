#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"
#include "lights_events.h"
#include "lights_strobe.h"

#define TEST_LIGHTS_STROBE_DURATION 300

static LightsStrobe s_lights_strobe = { 0 };
static LightsBlinkerDuration s_duration_ms = TEST_LIGHTS_STROBE_DURATION;

void setup_test(void) {
  event_queue_init();
  lights_strobe_init(&s_lights_strobe, s_duration_ms);
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

// Strobe must be initialized to off.
void test_lights_strobe_init() {
  LightsStrobe strobe = { 0 };
  TEST_ASSERT_OK(lights_strobe_init(&strobe, s_duration_ms));
  TEST_ASSERT_EQUAL(strobe.state, LIGHTS_STROBE_STATE_OFF);
}

void test_lights_strobe_turn_on_off() {
  // Turning strobe on.
  const Event e1 = {
    .id = LIGHTS_EVENT_STROBE,      //
    .data = LIGHTS_STROBE_STATE_ON  //
  };
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_STROBE_BLINK_ON);

  // Turning strobe off.
  const Event e2 = {
    .id = LIGHTS_EVENT_STROBE,       //
    .data = LIGHTS_STROBE_STATE_OFF  //
  };
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e2));
  prv_assert_raised_event(LIGHTS_EVENT_STROBE_BLINK_OFF);
}

// Do nothing when event is not LIGHTS_EVENT_STROBE
void test_lights_strobe_non_strobe_event() {
  // Left signal event.
  const Event e1 = {
    .id = LIGHTS_EVENT_SIGNAL_LEFT,  //
    .data = LIGHTS_STROBE_STATE_ON   //
  };
  // Do nothing.
  TEST_ASSERT_OK(lights_strobe_process_event(&s_lights_strobe, &e1));
  prv_assert_no_event_raised();
}

void teardown_test() {}
