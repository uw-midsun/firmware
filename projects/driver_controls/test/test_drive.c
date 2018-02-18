#include "unity.h"
#include "event_arbiter.h"
#include "soft_timer.h"
#include "interrupt.h"
#include "delay.h"
#include "pedal_fsm.h"
#include "direction_fsm.h"
#include "drive_output.h"
#include "can.h"
#include "event_queue.h"
#include "input_event.h"
#include "power_fsm.h"
#include "log.h"
#include "test_helpers.h"

// Tests interaction between the drive output module and power/direction/pedal FSMs

typedef enum {
  TEST_DRIVE_FSM_POWER = 0,
  TEST_DRIVE_FSM_PEDAL,
  TEST_DRIVE_FSM_DIRECTION,
  NUM_TEST_DRIVE_FSMS
} TestDriveFsm;

EventArbiterStorage s_arbiter_storage;
static FSM s_fsms[NUM_TEST_DRIVE_FSMS];

static void prv_clock_update_request(void) {
  Event e = { 0 };
  StatusCode ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, e.id);
  bool transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
  TEST_ASSERT_TRUE(transitioned);
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  drive_output_init(drive_output_global(), INPUT_EVENT_DRIVE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);
  event_arbiter_init(&s_arbiter_storage);
  power_fsm_init(&s_fsms[TEST_DRIVE_FSM_POWER], &s_arbiter_storage);
  pedal_fsm_init(&s_fsms[TEST_DRIVE_FSM_PEDAL], &s_arbiter_storage);
  direction_fsm_init(&s_fsms[TEST_DRIVE_FSM_DIRECTION], &s_arbiter_storage);
}

void teardown_test(void) {

}

void test_drive_basic(void) {
  Event e = { 0 };
  bool transitioned = false;

  // Try sending some drive commands before power on
  LOG_DEBUG("Raising events before power on\n");
  event_raise(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, 0);
  event_raise(INPUT_EVENT_TURN_SIGNAL_LEFT, 0);
  event_raise(INPUT_EVENT_PEDAL_PRESSED, 0);

  while (status_ok(event_process(&e))) {
    transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
    TEST_ASSERT_FALSE(transitioned);
  }

  // Send the correct sequence of events to enter power on
  LOG_DEBUG("Powering on\n");
  event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, 0);
  event_raise(INPUT_EVENT_POWER, 0);

  while (status_ok(event_process(&e))) {
    LOG_DEBUG("processing event %d\n", e.id);
    transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
    TEST_ASSERT_TRUE(transitioned);
    LOG_DEBUG("power FSM now in state %s\n", s_fsms[TEST_DRIVE_FSM_POWER].current_state->name);
  }

  // Power should now be on - process events before the watchdog faults
  LOG_DEBUG("Expecting drive update requests\n");
  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);

  // Should be an update request
  prv_clock_update_request();

  // Try changing the data and waiting for another output
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
  TEST_ASSERT_TRUE(transitioned);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  // TODO: add CAN handler to validate data

  // Go forward
  e.id = INPUT_EVENT_PEDAL_PRESSED;
  transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
  TEST_ASSERT_TRUE(transitioned);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  // Enter cruise
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
  TEST_ASSERT_TRUE(transitioned);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  // Exit using brake
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  transitioned = event_arbiter_process_event(&s_arbiter_storage, &e);
  TEST_ASSERT_TRUE(transitioned);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
}
