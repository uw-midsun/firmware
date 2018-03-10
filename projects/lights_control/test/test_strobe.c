#include "event_queue.h"
#include "test_helpers.h"
#include "soft_timer.h"
#include "unity.h"

#include "strobe.h"
#include "lights_events.h"

#define DURATION 500

#define INVALID_CALLBACK_ARG 2

static uint16_t s_strobe_cb_arg = INVALID_CALLBACK_ARG;

static StatusCode prv_strobe_callback(Event e) {
  switch (e.id) {
    case EVENT_SIGNAL_LEFT:
    case EVENT_SIGNAL_RIGHT:
    case EVENT_SIGNAL_HAZARD:
    case EVENT_SYNC:
    case EVENT_HORN:
    case EVENT_HEADLIGHTS:
    case EVENT_BRAKES:
      TEST_FAIL();
  }
  s_strobe_cb_arg = e.data;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  soft_timer_init();
  TEST_ASSERT_OK(strobe_init(prv_strobe_callback, DURATION));

}

void teardown_test(void) {
}

void test_strobe_event_process(void) {
  Event e0 = { .id = EVENT_HORN, .data = 0 };
  TEST_ASSERT_OK(strobe_event_process(e0));

  Event e1 = { .id = EVENT_STROBE, .data = 1 };
  TEST_ASSERT_OK(strobe_event_process(e1));
  TEST_ASSERT_EQUAL(s_strobe_cb_arg, STROBE_STATE_ON);

  s_strobe_cb_arg = INVALID_CALLBACK_ARG;
  Event e2 = { .id = EVENT_STROBE, .data = 0 };
  TEST_ASSERT_OK(strobe_event_process(e2));
  TEST_ASSERT_EQUAL(s_strobe_cb_arg, STROBE_STATE_OFF);
}

