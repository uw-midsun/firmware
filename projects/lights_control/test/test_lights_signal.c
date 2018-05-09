#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"
#include "lights_events.h"
#include "lights_signal_fsm.h"

#define TEST_LIGHTS_SIGNAL_FSM_DURATION 300

static LightsBlinkerDuration s_duration_ms = TEST_LIGHTS_SIGNAL_FSM_DURATION;

static LightsSignalFSM s_signal_fsm = { 0 };

void setup_test(void) {
  event_queue_init();
  lights_signal_fsm_init(&s_signal_fsm, s_duration_ms);
}

static void prv_assert_raised_event(LightsEvent event) {
  const Event raised_event = { 0 };
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(raised_event.id, event);
}

static void prv_assert_no_event_raised() {
  // Clearing event.
  const Event raised_event = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&raised_event));
}

// State none will transition to left, right, and hazard states.
void test_lights_signal_fsm_process_event_none_to_left_and_right(void) {
  // none -> left -> none -> right -> none

  // Go to left state
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_ON);

  // Can't transition to right state (invalid transition)
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_no_event_raised();

  // Go back to none state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_OFF };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_OFF);

  // Go to right state.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_RIGHT_BLINK_ON);

  // Can't go to left state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  prv_assert_no_event_raised();

  // Go back to none state.
  const Event e6 = { .id = LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_SIGNAL_FSM_CMD_OFF };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e6));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_RIGHT_BLINK_OFF);
}

void test_lights_signal_fsm_process_event_hazard_left_left(void) {
  // none -> hazard -> hazard-left -> left -> none

  // Go to hazard state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_HAZARD, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_HAZARD_BLINK_ON);

  // Go to hazard-left state (no change in behaviour).
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_no_event_raised();

  // Can't go to right state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_RIGHT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_no_event_raised();

  // Go to left state from hazard-left.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_HAZARD, .data = LIGHTS_SIGNAL_FSM_CMD_OFF };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_HAZARD_BLINK_OFF);
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_ON);

  // Go back to none state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_OFF };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_OFF);
}

void test_lights_signal_fsm_process_event_left_hazard_left(void) {
  // none -> left -> hazard-left -> hazard -> none

  // Go to left state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_ON);

  // Go to left-hazard state.
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_HAZARD, .data = LIGHTS_SIGNAL_FSM_CMD_ON };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_LEFT_BLINK_OFF);
  prv_assert_raised_event(LIGHTS_EVENT_SIGNAL_HAZARD_BLINK_ON);

  // Go to hazard state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_LEFT, .data = LIGHTS_SIGNAL_FSM_CMD_OFF };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_no_event_raised();
}

void teardown_test(void) {}
