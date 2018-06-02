#include <stdlib.h>

#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"
#include "lights_events.h"

#define TEST_LIGHTS_BLINKER_DURATION_SHORT 50
#define TEST_LIGHTS_BLINKER_DURATION_LONG 75
#define TEST_LIGHTS_BLINKER_INITIAL_COUNT 0
#define TEST_LIGHTS_BLINKER_BLINK_THRESHOLD 4

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_lights_blinker_init(void) {
  LightsBlinker blinker_1 = { 0 };
  lights_blinker_init(&blinker_1, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                      LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD);
  TEST_ASSERT_EQUAL(SOFT_TIMER_INVALID_TIMER, blinker_1.timer_id);
  LightsBlinker blinker_2 = { 0 };
  lights_blinker_init(&blinker_2, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                      TEST_LIGHTS_BLINKER_BLINK_THRESHOLD);
  TEST_ASSERT_EQUAL(TEST_LIGHTS_BLINKER_INITIAL_COUNT, blinker_2.blink_count);
  TEST_ASSERT_EQUAL(TEST_LIGHTS_BLINKER_BLINK_THRESHOLD, blinker_2.blink_count_threshold);
}

void test_lights_blinker_activate(void) {
  // Testing two blinkers with different times to make sure one happens after the other.
  LightsBlinker blinker_1 = { 0 };
  LightsBlinker blinker_2 = { 0 };

  TEST_ASSERT_OK(lights_blinker_init(&blinker_1, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_init(&blinker_2, TEST_LIGHTS_BLINKER_DURATION_LONG,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_1, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD));
  TEST_ASSERT_EQUAL(TEST_LIGHTS_BLINKER_DURATION_SHORT, blinker_1.duration_ms);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_1.state);
  TEST_ASSERT_NOT_EQUAL(SOFT_TIMER_INVALID_TIMER, blinker_1.timer_id);
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD, e.data);

  TEST_ASSERT_OK(lights_blinker_activate(&blinker_2, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);

  // Now we make sure blinker 1 timer goes off before blinker 2
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD, e.data);

  // Wait for the second blinker
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
}

void test_lights_blinker_activate_already_active(void) {
  // Activating an already-active blinker will result in deactivating the old blinker and
  // activating a new one.
  LightsBlinker blinker = { 0 };
  Event e = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT));
  TEST_ASSERT_OK(event_process(&e));
  // Since two events (ON, and OFF) are raised at the same time, we need to make sure the OFF event
  // takes precedence over the ON event.
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
}

void test_lights_blinker_activate_with_existing_peripheral(void) {
  // Activating an already-active blinker with the same peripheral will have no effect on the
  // behaviour.
  LightsBlinker blinker = { 0 };
  Event e = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_NOT_OK(event_process(&e));
}

void test_lights_blinker_activate_existing_peripheral_while_inactive(void) {
  // If lights_blinker_activate gets called with the same peripheral, we're not expecting any
  // behaviour change. But if the blinker is already inactive, there should be a behaviour change.
  LightsBlinker blinker = { 0 };
  Event e = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  // Activating with signal left peripheral.
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  // Activating again with the same peripheral. No event should get raised.
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_NOT_OK(event_process(&e));
  // Deactivating the blinker. A LIGHTS_EVENT_GPIO_OFF should generate.
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
  // Activating with the same peripheral again. It should raise an event.
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
}

void test_lights_blinker_deactivate_timer_cancelled(void) {
  // Making sure timer gets cancelled after we deactivate the blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_TRUE(soft_timer_inuse());
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  TEST_ASSERT_EQUAL(SOFT_TIMER_INVALID_TIMER, blinker.timer_id);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_lights_blinker_deactivate_already_inactive(void) {
  // lights_blinker_deactivate requires that the blinker be an active blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  StatusCode s = lights_blinker_deactivate(&blinker);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s);
}

void test_lights_blinker_force_on_update(void) {
  // lights_blinker_force_on_update should reschedule the timer, and start over with an on state
  // again.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);

  // Syncing blinker.
  TEST_ASSERT_OK(lights_blinker_force_on(&blinker));
  TEST_ASSERT_OK(event_process(&e));
  // Raised event must be on again. If we didn't call lights_blinker_force_on, event would not get
  // raised immediately, and would have been an off blink event.
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
}

void test_lights_blinker_force_on_initialized_not_activated(void) {
  // Initialized blinkers are inactive by default. They cannot be synced to ON state.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  StatusCode s = lights_blinker_force_on(&blinker);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s);
}

void test_lights_blinker_force_on_while_deactivated(void) {
  // We should not be able to force_on a deactivated blinker.
  LightsBlinker blinker = { 0 };
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT,
                                     LIGHTS_BLINKER_NON_SYNCING_COUNT_THRESHOLD));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  // Wait for a full blink cycle.
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT, e.data);
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);

  TEST_ASSERT_OK(lights_blinker_deactivate(&blinker));
  StatusCode s = lights_blinker_force_on(&blinker);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, s);
}

void test_lights_blinker_with_sync(void) {
  // Initialize a blinker with sync functionality.
  LightsBlinker blinker = { 0 };
  uint32_t count = 3;
  TEST_ASSERT_OK(lights_blinker_init(&blinker, TEST_LIGHTS_BLINKER_DURATION_SHORT, count));
  TEST_ASSERT_OK(lights_blinker_activate(&blinker, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT));
  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  for (uint8_t i = 0; i < count; i++) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_OFF, e.id);
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    TEST_ASSERT_EQUAL(LIGHTS_EVENT_GPIO_ON, e.id);
  }
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  // Expect the sync event to have been raised.
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
}
