#include "delay.h"
#include "drive_output.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_DRIVE_OUTPUT_EVENT_FAULT = 0,
  TEST_DRIVE_OUTPUT_EVENT_UPDATE_REQ,
} TestDriveOutputEvent;

static DriveOutputStorage s_storage;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  drive_output_init(&s_storage, TEST_DRIVE_OUTPUT_EVENT_FAULT, TEST_DRIVE_OUTPUT_EVENT_UPDATE_REQ);
}

void teardown_test(void) {}

void test_drive_output_working(void) {
  drive_output_set_enabled(&s_storage, true);

  for (size_t i = 0; i < NUM_DRIVE_OUTPUT_SOURCES; i++) {
    drive_output_update(&s_storage, i, i * 100);
  }

  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // Should not have raised a fault event
  Event e = { 0 };
  while (status_ok(event_process(&e))) {
    TEST_ASSERT_EQUAL(TEST_DRIVE_OUTPUT_EVENT_UPDATE_REQ, e.id);
  }

  drive_output_set_enabled(&s_storage, false);

  // Make sure that we don't raise any events after console output has been disabled
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS * 2);
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // Reenable and fault
  drive_output_set_enabled(&s_storage, true);
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);
  ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(TEST_DRIVE_OUTPUT_EVENT_FAULT, e.id);
}
