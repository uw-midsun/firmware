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

#define TEST_LIGHTS_BLINKER_DURATION_SHORT 300
#define TEST_LIGHTS_BLINKER_DURATION_LONG 500

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_lights_blinker_init(void) {
  LightsBlinker blinker = { 0 };
  lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT);
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
}

void test_lights_blinker_activate(void) {
  // Testing two blinkers with different times to make sure one happens after the other.
  LightsBlinker blinker_1 = { 0 };
  LightsBlinker blinker_2 = { 0 };

  TEST_ASSERT_OK(lights_blinker_init(&blinker_1, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_init(&blinker_2, TEST_LIGHTS_BLINKER_DURATION_LONG));

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_1, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_EQUAL(blinker_1.duration_ms, TEST_LIGHTS_BLINKER_DURATION_SHORT);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_1.state);
  TEST_ASSERT_NOT_EQUAL(blinker_1.timer_id, SOFT_TIMER_INVALID_TIMER);

  Event e = { 0 };
  // Wait for the blinker_1 initial event to fire. This happens almost instantly as the event gets
  // raised upon call to lights_blinker_activate.
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_2, LIGHTS_BLINKER_TEST_EVENT_2));
  TEST_ASSERT_EQUAL(blinker_2.duration_ms, TEST_LIGHTS_BLINKER_DURATION_LONG);
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

void test_lights_blinker_activate_already_active(void) {
  // lights_blinker_activate requires that the blinker be inactive before calling.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_NOT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));
}

void test_lights_blinker_deactivate(void) {
  // Making sure timer gets cancelled after we deactivate the blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
}

void test_lights_blinker_deactivate_already_inactive(void) {
  // lights_blinker_deactivate requires that the blinker be an active blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_deactivate(&blinker));
}

void test_lights_blinker_sync_update(void) {
  // lights_blinker_sync_update should reschedule the timer, and start over with an on state again.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));

  Event e = { 0 };
  while (event_process(&e) != STATUS_CODE_OK) {
  }

  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_sync_on(&blinker));

  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_BLINKER_TEST_EVENT_1);
  // Raised event must be on again. If we didn't call lights_blinker_sync_on, this would have been an off blink
  // event.
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);
}

void test_lights_blinker_sync_on_initialized_not_activated(void) {
  // Initialized blinkers are inactive by default. They cannot be synced to ON state.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_NOT_OK(lights_blinker_sync_on(&blinker));
}

void test_lights_blinker_sync_on_while_deactivated(void) {
  // We should not be able to sync_on a deactivated blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_BLINKER_TEST_EVENT_1));
  // Wait for a full blink cycle.
  Event e = { 0 };
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_sync_on(&blinker));
}

