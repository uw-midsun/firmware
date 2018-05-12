#include <stdio.h>
#include <stdlib.h>

#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"
#include "lights_events.h"

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

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_1, LIGHTS_EVENT_GPIO_DATA_SIGNAL_HAZARD));
  TEST_ASSERT_EQUAL(blinker_1.duration_ms, TEST_LIGHTS_BLINKER_DURATION_SHORT);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_1.state);
  TEST_ASSERT_NOT_EQUAL(blinker_1.timer_id, SOFT_TIMER_INVALID_TIMER);
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_HAZARD);

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_2, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);

  // Now we make sure blinker 1 timer goes off before blinker 2
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_OFF);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_HAZARD);

  // Wait for the second blinker
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_OFF);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
}

void test_lights_blinker_activate_already_active(void) {
  // Activating an already-active blinker will result in deactivating the old blinker and
  // activating a new one.
  LightsBlinker blinker = { 0 };
  Event e = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_RIGHT));
  TEST_ASSERT_OK(event_process(&e));
  // Since two events (ON, and OFF) are raised at the same time, we need to make sure the OFF event 
  // takes precedence over the ON event.
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_OFF);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_RIGHT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
}

void test_lights_blinker_activate_with_existing_event_data(void) {
  // Activating an already-active blinker with the same event data will have no effect on the
  // behaviour.
  LightsBlinker blinker = { 0 };
  Event e = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_NOT_OK(event_process(&e));
}

void test_lights_blinker_deactivate_timer_cancelled(void) {
  // Making sure timer gets cancelled after we deactivate the blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_TRUE(soft_timer_inuse());
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_lights_blinker_deactivate_already_inactive(void) {
  // lights_blinker_deactivate requires that the blinker be an active blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_deactivate(&blinker));
}

void test_lights_blinker_sync_update(void) {
  // lights_blinker_sync_update should reschedule the timer, and start over with an on state again.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);

  // Syncing blinker.
  TEST_ASSERT_OK(lights_blinker_sync_on(&blinker));
  TEST_ASSERT_OK(event_process(&e));
  // Raised event must be on again. If we didn't call lights_blinker_sync_on, event would not get
  // raised immediately, and would have been an off blink event.
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
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
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT));
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_ON);
  // Wait for a full blink cycle.
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.data, LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT);
  TEST_ASSERT_EQUAL(e.id, LIGHTS_EVENT_GPIO_OFF);

  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_sync_on(&blinker));
}
