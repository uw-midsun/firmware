#include "drive_output.h"

#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static DriveOutputStorage s_storage;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  drive_output_init(&s_storage, PEDAL_EVENT_INPUT_PEDAL_WATCHDOG_FAULT,
                    PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED);
}

void teardown_test(void) {}

void test_drive_output_working(void) {
  drive_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_DRIVE_OUTPUT_SOURCES; i++) {
    drive_output_update(&s_storage, i, i * 100);
  }

  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // We should keep on updating the Drive Output messages
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED, e.id);
  }

  drive_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after drive output has been disabled
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);
}

void test_drive_output_watchdog(void) {
  drive_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < (NUM_DRIVE_OUTPUT_SOURCES - 1); i++) {
    // Update all sources but one
    drive_output_update(&s_storage, i, i * 100);
  }

  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // Should have raised a fault event since we were missing a source
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    if (e.id == PEDAL_EVENT_INPUT_PEDAL_WATCHDOG_FAULT) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(PEDAL_EVENT_INPUT_PEDAL_WATCHDOG_FAULT, e.id);
}
