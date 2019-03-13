#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "steering_output.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_STEERING_OUTPUT_EVENT_FAULT = 0,
  TEST_STEERING_OUTPUT_EVENT_UPDATE_REQ,
} TestSteeringOutputEvent;

static SteeringOutputStorage s_storage;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  steering_output_init(&s_storage, TEST_STEERING_OUTPUT_EVENT_FAULT,
                       TEST_STEERING_OUTPUT_EVENT_UPDATE_REQ);
}

void teardown_test(void) {}

void test_steering_output_working(void) {
  steering_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_STEERING_OUTPUT_SOURCES; i++) {
    steering_output_update(&s_storage, i, i * 100);
  }

  delay_ms(STEERING_OUTPUT_WATCHDOG_MS);

  // Should not have raised a fault event
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(TEST_STEERING_OUTPUT_EVENT_UPDATE_REQ, e.id);
  }

  steering_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after steering output has been disabled
  delay_ms(STEERING_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // Reenable and fault
  steering_output_set_enabled(&s_storage, true);
  delay_ms(STEERING_OUTPUT_WATCHDOG_MS);
  ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(TEST_STEERING_OUTPUT_EVENT_FAULT, e.id);
}

void test_steering_output_watchdog(void) {
  steering_output_set_enabled(&s_storage, true);

  // Don't update the single output source
  delay_ms(STEERING_OUTPUT_WATCHDOG_MS);

  // Should have raised a fault event since we were missing a source
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    if (e.id == TEST_STEERING_OUTPUT_EVENT_FAULT) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(TEST_STEERING_OUTPUT_EVENT_FAULT, e.id);
}
