#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"
#include "lights_events.h"
#include "lights_signal_fsm.h"

#define TEST_LIGHTS_SIGNAL_FSM_DURATION 300

typedef enum {
  TEST_LIGHTS_SIGNAL_FSM_CMD_OFF = 0,
  TEST_LIGHTS_SIGNAL_FSM_CMD_ON,
  NUM_TEST_LIGHTS_SIGNAL_FSM_CMDS
} TestLightsSignalFsmCmd;

static LightsBlinkerDuration s_duration_ms = TEST_LIGHTS_SIGNAL_FSM_DURATION;

static LightsSignalFsm s_signal_fsm = { 0 };

void setup_test(void) {
  event_queue_init();
  lights_signal_fsm_init(&s_signal_fsm, s_duration_ms);
}

static void prv_assert_raised_event(LightsEvent event, LightsEventGpioPeripheral peripheral) {
  const Event raised_event = { 0 };
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(peripheral, raised_event.data);
  TEST_ASSERT_EQUAL(event, raised_event.id);
}

static void prv_assert_no_event_raised() {
  // Clearing event.
  const Event raised_event = { 0 };
  TEST_ASSERT_NOT_OK(event_process(&raised_event));
}

// State none will transition to left, right, and hazard states.
void test_lights_signal_fsm_process_event_none_to_left_and_right(void) {
  // State transitions: none -> left -> none -> right -> none

  // Go to left state
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Can't transition to right state (invalid transition)
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_no_event_raised();

  // Go back to none state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go to right state.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);

  // Can't go to left state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  prv_assert_no_event_raised();

  // Go back to none state.
  const Event e6 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e6));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
}

void test_lights_signal_fsm_process_event_hazard_left_left(void) {
  // State transitions: none -> hazard -> hazard-left -> left -> none
  // Go to hazard state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);

  // Go to hazard-left state (no change in behaviour).
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_no_event_raised();

  // Can't go to right state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_no_event_raised();

  // Go to left state from hazard-left.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go back to none state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
}

void test_lights_signal_fsm_process_event_left_hazard_left(void) {
  // State transitions: none -> left -> hazard-left -> hazard -> none

  // Go to left state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go to left-hazard state.
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  prv_assert_raised_event(LIGHTS_EVENT_GPIO_ON, LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);

  // Go to hazard state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  prv_assert_no_event_raised();
}

void teardown_test(void) {}
