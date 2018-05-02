#include <stdlib.h>

#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"

typedef enum {
  LIGHTS_BLINKER_TEST_EVENT_1 = 0,
  LIGHTS_BLINKER_TEST_EVENT_2,
  NUM_LIGHTS_TEST_EVENTS
} LightsTestEvent;

#define DURATION_SHORT 300
#define DURATION_LONG 500

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_lights_blinker_init(void) {
  LightsBlinker blinker;
  lights_blinker_init(&blinker);
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
}

void test_lights_blinker_on(void) {
  // Testing two blinkers with different times to make sure one happens after the other.
  LightsBlinker blinker_1;
  LightsBlinker blinker_2;

  TEST_ASSERT_OK(lights_blinker_init(&blinker_1));
  TEST_ASSERT_OK(lights_blinker_init(&blinker_2));

  TEST_ASSERT_OK(lights_blinker_on(&blinker_1, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_EQUAL(blinker_1.duration_ms, DURATION_SHORT);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_1.state);
  TEST_ASSERT_NOT_EQUAL(blinker_1.timer_id, SOFT_TIMER_INVALID_TIMER);

  Event e;
  // Wait for the blinker_1 initial event to fire. This happens almost instantly as the event gets
  // raised upon call to lights_blinker_on.
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_on(&blinker_2, DURATION_LONG, LIGHTS_BLINKER_TEST_EVENT_2));
  TEST_ASSERT_EQUAL(blinker_2.duration_ms, DURATION_LONG);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_2.state);
  TEST_ASSERT_NOT_EQUAL(blinker_2.timer_id, SOFT_TIMER_INVALID_TIMER);

  // Wait for the blinker_2 initial event to fire.
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_2);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  // Now we make sure blinker 1 timer goes off before blinker 2
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);

  // Wait for the second blinker
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_2);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);
}

void test_lights_blinker_on_uninitialized(void) {
  // We shoudln't be able to call blinker_on on an uninitialized blinker, as uninitialized blinkers
  // might have timer_id's that may be valid timer id's and may be in use by other modules in the
  // application.
  LightsBlinker blinker;
  TEST_ASSERT_NOT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
}

void test_lights_blinker_on_already_active(void) {
  // lights_blinker_on requires that the blinker be inactive before calling.
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_NOT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
}

void test_lights_blinker_off(void) {
  // Making sure timer gets cancelled after we turn the blinker off
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_lights_blinker_off_already_inactive(void) {
  // lights_blinker_off requires that the blinker be an active blinker
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_off(&blinker));
}

void test_lights_blinker_reset(void) {
  // lights_blinker_reset should reschedule the timer, and start over with an on state again.
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));

  Event e;
  while (event_process(&e) != STATUS_CODE_OK) {
  }

  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_reset(&blinker));

  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  // Raised event must be on again. If we didn't call reset, this would have been an off blink
  // event.
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);
}

void test_lights_blinker_reset_while_off(void) {
  // We should not be able to reset an off blinker.
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  // Wait for a full blink cycle.
  Event e;
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_reset(&blinker));
}

void test_lights_blinker_inuse(void) {
  LightsBlinker blinker;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on(&blinker, DURATION_SHORT, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_TRUE(lights_blinker_inuse(&blinker));
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_FALSE(lights_blinker_inuse(&blinker));
}
