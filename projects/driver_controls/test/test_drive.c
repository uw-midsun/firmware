#include "can.h"
#include "cruise_fsm.h"
#include "delay.h"
#include "direction_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "hazards_fsm.h"
#include "headlight_fsm.h"
#include "horn_fsm.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "turn_signal_fsm.h"
#include "unity.h"

// Tests interaction between the drive output module and power/direction/pedal FSMs

#define TEST_DRIVE_CLOCK_EVENT(event_id, should_succeed)                                      \
  {                                                                                           \
    Event e = { .id = (event_id) };                                                           \
    TEST_ASSERT_EQUAL((should_succeed), event_arbiter_process_event(&s_arbiter_storage, &e)); \
    if ((should_succeed) &&                                                                   \
        (e.id == INPUT_EVENT_CENTER_CONSOLE_POWER || e.id == INPUT_EVENT_BPS_FAULT ||         \
         e.id == INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE ||                                \
         e.id == INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL ||                              \
         e.id == INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE ||                              \
         e.id == INPUT_EVENT_MECHANICAL_BRAKE_PRESSED ||                                      \
         e.id == INPUT_EVENT_MECHANICAL_BRAKE_RELEASED)) {                                    \
      if (status_ok(event_process(&e))) {                                                     \
        event_arbiter_process_event(&s_arbiter_storage, &e);                                  \
      }                                                                                       \
    }                                                                                         \
  }

typedef enum {
  TEST_DRIVE_FSM_POWER = 0,
  TEST_DRIVE_FSM_CRUISE,
  TEST_DRIVE_FSM_PEDAL,
  TEST_DRIVE_FSM_DIRECTION,
  TEST_DRIVE_FSM_MECH_BRAKE,
  TEST_DRIVE_FSM_HEADLIGHT,
  TEST_DRIVE_FSM_TURN_SIGNALS,
  TEST_DRIVE_FSM_HAZARDS,
  TEST_DRIVE_FSM_HORN,
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
    printf("> %-30s%s\n", s_fsms[i].name, s_fsms[i].current_state->name);
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
  cruise_fsm_init(&s_fsms[TEST_DRIVE_FSM_CRUISE], &s_arbiter_storage);
  pedal_fsm_init(&s_fsms[TEST_DRIVE_FSM_PEDAL], &s_arbiter_storage);
  direction_fsm_init(&s_fsms[TEST_DRIVE_FSM_DIRECTION], &s_arbiter_storage);
  mechanical_brake_fsm_init(&s_fsms[TEST_DRIVE_FSM_MECH_BRAKE], &s_arbiter_storage);

  turn_signal_fsm_init(&s_fsms[TEST_DRIVE_FSM_TURN_SIGNALS], &s_arbiter_storage);
  horn_fsm_init(&s_fsms[TEST_DRIVE_FSM_HORN], &s_arbiter_storage);
  hazards_fsm_init(&s_fsms[TEST_DRIVE_FSM_HAZARDS], &s_arbiter_storage);
  headlight_fsm_init(&s_fsms[TEST_DRIVE_FSM_HEADLIGHT], &s_arbiter_storage);
}

void teardown_test(void) {}

void test_drive_basic(void) {
  Event e = { 0 };
  bool transitioned = false;

  // Try sending some drive commands before power on
  LOG_DEBUG("Raising events before power on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, false);

  // Send the correct sequence of events to enter power on
  LOG_DEBUG("Powering on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);

  LOG_DEBUG("Moving direction to reverse\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE, true);

  // Power should now be on - process events before the watchdog faults
  LOG_DEBUG("Expecting drive update requests\n");
  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);

  // Should be an update request - should have brake on
  prv_clock_update_request();

  LOG_DEBUG("Moving direction to drive\n");
  // Try changing the direction (mech brake still held) and waiting for another output
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  LOG_DEBUG("Releasing mechanical brake and regen brake\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);

  // Note that we no longer block pedal transitions - it's up to motor controller interface to
  // protect against mechanical brake/throttle

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  LOG_DEBUG("Entering cruise control\n");
  // Enter cruise
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();

  LOG_DEBUG("Exiting cruise control through mechanical brake\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  prv_dump_fsms();

  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();
}

void test_drive_charge(void) {
  Event e = { 0 };

  // Move to charging
  LOG_DEBUG("Moving to charging state\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);

  // Check charging behavior - make sure that drive commands are not sent
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // No update or fault event should be raised
  LOG_DEBUG("Ensuring that no drive commands were sent\n");
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);
}

void test_drive_cruise(void) {
  LOG_DEBUG("Moving to powered drive\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);

  LOG_DEBUG("Attempting to enter cruise from braking state\n");
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, false);

  // The cruise module will never support cruising in the negative velocity
  LOG_DEBUG("Attempt to enter cruise from reverse\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  // pretend we're accelerating
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, false);

  // Move to coast
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_COAST, true);

  // Check cruise exits
  LOG_DEBUG("Entering cruise\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  LOG_DEBUG("Exiting cruise and rentering\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, true);
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  LOG_DEBUG("Exiting cruise through mechanical brake and reentering\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  LOG_DEBUG("Exiting cruise through accel - braking\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, true);
  prv_dump_fsms();
  LOG_DEBUG("Accelerating\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);
  prv_dump_fsms();
}

void test_drive_fault(void) {
  Event e;

  LOG_DEBUG("Moving to powered drive\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);

  LOG_DEBUG("Raising fault\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_BPS_FAULT, true);

  // Fault occurred - almost all events are disabled
  LOG_DEBUG("Raising a bunch of events\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, false);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE, false);

  // Make sure that drive commands are not sent
  delay_ms(DRIVE_OUTPUT_WATCHDOG_MS);

  // No update or fault event should be raised
  LOG_DEBUG("Ensuring that no drive commands were sent\n");
  StatusCode ret = event_process(&e);
  TEST_ASSERT_NOT_OK(ret);

  LOG_DEBUG("Clearing fault\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);

  LOG_DEBUG("Powering on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  prv_dump_fsms();

  LOG_DEBUG("Checking for new drive commands\n");
  delay_ms(DRIVE_OUTPUT_BROADCAST_MS);
  prv_clock_update_request();
}

void test_drive_revert_cruise(void) {
  LOG_DEBUG("Moving to powered drive\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);

  LOG_DEBUG("Entering cruise control\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  LOG_DEBUG("Turning car off\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  prv_dump_fsms();
  LOG_DEBUG("Turning car back on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);

  LOG_DEBUG("We should be able to enter cruise control (post-brake)\n");
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_BRAKE, true);

  LOG_DEBUG("Faulting\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_BPS_FAULT, true);

  LOG_DEBUG("Acknowledging fault\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);

  LOG_DEBUG("Turning car back on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);

  LOG_DEBUG("We should be able to enter cruise\n");
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);
}

void test_drive_revert_direction(void) {
  LOG_DEBUG("Moving to powered drive\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_PEDAL_ACCEL, true);

  LOG_DEBUG("We should be able to enter cruise control\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, true);

  LOG_DEBUG("Turning car off\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);

  LOG_DEBUG("Turning car back on\n");
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CENTER_CONSOLE_POWER, true);
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, true);

  LOG_DEBUG("Attempting to enter cruise control - we should be in neutral\n");
  prv_dump_fsms();
  TEST_DRIVE_CLOCK_EVENT(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, false);
}
