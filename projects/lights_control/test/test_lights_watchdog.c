#include <stdint.h>

#include "event_queue.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#include "exported_enums.h"
#include "lights_events.h"
#include "lights_watchdog.h"

#define TEST_LIGHTS_WATCHDOG_BEFORE_TIMEOUT_MS 150
#define TEST_LIGHTS_WATCHDOG_TIMEOUT_MS 200
#define TEST_LIGHTS_WATCHDOG_AFTER_TIMEOUT_MS 300

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

static void prv_timer_callback_raise_event(SoftTimerID timer_id, void *context) {
  Event *event = (Event *)context;
  event_raise(event->id, event->data);
}

// Watchdog should fire if it doesn't receive an event before its timeout duration.
void test_lights_watchdog_fire(void) {
  LightsWatchDogStorage storage = { 0 };
  // Initializing watchdog.
  TEST_ASSERT_OK(lights_watchdog_init(&storage, TEST_LIGHTS_WATCHDOG_TIMEOUT_MS));
  // Scheduling a late event (this event must occur after the watchdog's timer)
  Event late_event = { .id = LIGHTS_EVENT_BPS_HEARTBEAT, .data = EE_BPS_HEARTBEAT_STATE_OK };
  SoftTimerID late_timer_id = 0;
  TEST_ASSERT_OK(soft_timer_start_millis(TEST_LIGHTS_WATCHDOG_AFTER_TIMEOUT_MS,
                                         prv_timer_callback_raise_event, &late_event,
                                         &late_timer_id));

  // Expect watchdog to fire.
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_STROBE_ON, e.id);

  // Expect the late event to arrive.
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_BPS_HEARTBEAT, e.id);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, e.data);

  // Timer shouldn't be active after watchdog has fired.
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

// Watchdog should fire if the bps heartbeat event has error state.
void test_lights_bps_error_watchdog_fire(void) {
  LightsWatchDogStorage storage = { 0 };
  TEST_ASSERT_OK(lights_watchdog_init(&storage, TEST_LIGHTS_WATCHDOG_TIMEOUT_MS));
  Event fail_event = { .id = LIGHTS_EVENT_BPS_HEARTBEAT, .data = EE_BPS_HEARTBEAT_STATE_ERROR };
  SoftTimerID fail_timer_id = 0;
  TEST_ASSERT_OK(soft_timer_start_millis(TEST_LIGHTS_WATCHDOG_BEFORE_TIMEOUT_MS,
                                         prv_timer_callback_raise_event, &fail_event,
                                         &fail_timer_id));

  // Expect fail event to arrive before watchdog timeout.
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_BPS_HEARTBEAT, e.id);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_ERROR, e.data);

  // Passing the event to lights_watchdog.
  TEST_ASSERT_OK(lights_watchdog_process_event(&storage, &e));

  // Expect the watchdog to fire immediately.
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_STROBE_ON, e.id);

  // Timer shouldn't be active after watchdog has fired.
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

// Watchdog should not fire if it receives a heartbeat event.
void test_lights_watchdog_not_fire(void) {
  // Timeline:
  // 0ms: Watchdog initializes
  // 150ms: In-time heartbeat event arrives and watchdog reschedules timer.
  // 200ms: Watchdog should not fire because it's rescheduled!
  // 300ms: Late arbitrary event arrives. This is to make sure watchdog happens after 300ms.
  // 350ms: Watchdog fires because it hasn't received any other heartbeat events.

  // Initializing watchdog.
  LightsWatchDogStorage storage = { 0 };
  TEST_ASSERT_OK(lights_watchdog_init(&storage, TEST_LIGHTS_WATCHDOG_TIMEOUT_MS));
  // Scheduling a heartbeat event.
  Event heartbeat = { .id = LIGHTS_EVENT_BPS_HEARTBEAT, .data = EE_BPS_HEARTBEAT_STATE_OK };
  SoftTimerID heartbeat_timer_id = 0;
  TEST_ASSERT_OK(soft_timer_start_millis(TEST_LIGHTS_WATCHDOG_BEFORE_TIMEOUT_MS,
                                         prv_timer_callback_raise_event, &heartbeat,
                                         &heartbeat_timer_id));
  // Scheduling a late arbitrary event.
  Event late_arbitrary_event = { .id = LIGHTS_EVENT_SYNC, .data = 0 };
  SoftTimerID late_timer_id = 0;
  TEST_ASSERT_OK(soft_timer_start_millis(TEST_LIGHTS_WATCHDOG_AFTER_TIMEOUT_MS,
                                         prv_timer_callback_raise_event, &late_arbitrary_event,
                                         &late_timer_id));

  // Expect heartbeat event to arrive before watchdog timeout.
  Event e = { 0 };
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_BPS_HEARTBEAT, e.id);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, e.data);
  TEST_ASSERT_OK(lights_watchdog_process_event(&storage, &e));

  // Expect late event to arrive before watchdog timeout. This is to make sure watchdog is
  // rescheduled.
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_SYNC, e.id);
  // Unrelated events should be ignored.
  TEST_ASSERT_OK(lights_watchdog_process_event(&storage, &e));

  // Expect watchdog to fire since there aren't any other heartbeat events sent to it.
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(LIGHTS_EVENT_STROBE_ON, e.id);

  // Timer shouldn't be active after watchdog has fired.
  TEST_ASSERT_FALSE(soft_timer_inuse());
}
