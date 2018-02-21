#include "can.h"
#include "delay.h"
#include "direction_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Tests interaction between the drive output module and power/direction/pedal FSMs

#define TEST_DRIVE_CLOCK_EVENT(event_id, should_succeed) \
  { \
    Event e = { .id = event_id }; \
    TEST_ASSERT_EQUAL(should_succeed, event_arbiter_process_event(&s_arbiter_storage, &e)); \
  }

typedef enum {
  TEST_DRIVE_FSM_POWER = 0,
  TEST_DRIVE_FSM_PEDAL,
  TEST_DRIVE_FSM_DIRECTION,
  TEST_DRIVE_FSM_MECH_BRAKE,
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

static void prv_dump_fsms(void) {
  for (size_t i = 0; i < NUM_TEST_DRIVE_FSMS; i++) {
    LOG_DEBUG("%s: %s\n", s_fsms[i].name, s_fsms[i].current_state->name);
  }
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
  mechanical_brake_fsm_init(&s_fsms[TEST_DRIVE_FSM_MECH_BRAKE], &s_arbiter_storage);
}

void teardown_test(void) {}

void test_drive_basic(void) {
  Event e = { 0 };
  bool transitioned = false;

  // Try sending some drive commands before power on
  LOG_DEBUG("Raising events before power on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_TURN_SIGNAL_LEFT, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_PRESSED, false);

  // Send the correct sequence of events to enter power on
  LOG_DEBUG("Powering on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, true);

  // Power should now be on - process events before the watchdog faults
  LOG_DEBUG("Expecting drive update requests\n");
  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);

  // Should be an update request - should have brake on
  prv_clock_update_request();

  LOG_DEBUG("Moving direction to drive\n");
  // Try changing the direction (mech brake still held) and waiting for another output
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  // TODO: add CAN handler to validate data

  LOG_DEBUG("Attempt to move forward with mechanical brake still held\n");
  // Go forward - fail due to mech brake still being held
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_PRESSED, false);

  LOG_DEBUG("Releasing mechanical brake\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);

  LOG_DEBUG("Moving forward\n");
  // Try again after releasing mech brake
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_PRESSED, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  LOG_DEBUG("Entering cruise control\n");
  // Enter cruise
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CRUISE_CONTROL, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  LOG_DEBUG("Exiting cruise control through mechanical brake\n");
  // Exit using brake - should be in brake state
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  // TODO: make sure we can't turn the car off if we're driving

  // TODO: should cruise control be associated with pedal? need to move to the appropriate state
  // when cruise control is disabled
}

void test_drive_charge(void) {
  // TODO: we should verify that receiving a "charging request" notification from the charger
  // puts us into the correct state
  Event e = { 0 };

  // Move to charging
  LOG_DEBUG("Moving to charging state\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, true);

  // Check charging behavior - make sure that drive commands are not sent
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // No update or fault event should be raised
  LOG_DEBUG("Ensuring that no drive commands were sent\n");
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  // TODO: make sure that charging state has been sent to power distribution
  // Make sure we don't allow any movement during charging
  LOG_DEBUG("Pressing the pedal should do nothing\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_PRESSED, false);
}

// Verifies basic interlocks
// goal: ensure that we can't power on or off in invalid situations
void test_drive_power_interlock(void) {
  Event e = { 0 };

  LOG_DEBUG("Moving to powered state\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, true);

  LOG_DEBUG("Releasing mechanical brake and starting to drive\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_PRESSED, true);

  LOG_DEBUG("Attempting to power off (should fail)\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, false);

  LOG_DEBUG("Switching into cruise and attempting to turn power off\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CRUISE_CONTROL, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, false);

  LOG_DEBUG("Exiting cruise and attempting to turn power off\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CRUISE_CONTROL, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_POWER, false);
  prv_dump_fsms();

  LOG_DEBUG("Attempting to switch to neutral using regen brake\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, false);
  prv_dump_fsms();
}

// Basically, the direction FSM is used to ensure that power is never switched off while the car is
// not in neutral
// The mechanical brake FSM is used to ensure that the direction FSM is only changed while
// mechanical brakes are active
// The pedal FSM is also switched into the brake state and held there

// Verify that cruise has the intended behavior
// TODO: add steering angle events?
void test_drive_cruise(void) {
  // todo: try to switch into reverse while cruise control is active
}
