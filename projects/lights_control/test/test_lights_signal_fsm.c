// This tests the signal FSM and makes sure the state transitions are properly being done.
// NOTE: The assertions work based on the raised event immediately after turning on a blinker.
// It is assumed that the soft-timer interrupt is not being triggered before the event assertion.

#include "lights_blinker.h"
#include "lights_events.h"
#include "lights_signal_fsm.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_LIGHTS_SIGNAL_DURATION 300

#define TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(event, peripheral) \
  do {                                                            \
    const Event raised_event = { 0 };                             \
    TEST_ASSERT_OK(event_process(&raised_event));                 \
    TEST_ASSERT_EQUAL(peripheral, raised_event.data);             \
    TEST_ASSERT_EQUAL(event, raised_event.id);                    \
  } while (0)

#define TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED() \
  do {                                              \
    const Event raised_event = { 0 };               \
    StatusCode s = event_process(&raised_event);    \
    TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, s);        \
  } while (0)

static LightsSignalFsm s_signal_fsm = { 0 };

void setup_test(void) {
  soft_timer_init();
  event_queue_init();
  lights_signal_fsm_init(&s_signal_fsm, TEST_LIGHTS_SIGNAL_DURATION,
                         LIGHTS_BLINKER_COUNT_THRESHOLD_NO_SYNC);
}

void teardown_test(void) {}

// State none will transition to left, right, and hazard states.
void test_lights_signal_fsm_process_event_none_to_left_and_right(void) {
  // State transitions: none -> left -> none -> right -> none

  // Go to left state
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  // Go back to none state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go to right state.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);

  // Go back to none state.
  const Event e6 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e6));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
}

void test_lights_signal_fsm_process_event_hazard_left_left(void) {
  // State transitions: none -> hazard -> hazard-left -> left -> none
  // Go to hazard state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);

  // Go to hazard-left state (no change in behaviour).
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // Go to left state from hazard-left.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go back to none state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
}

void test_lights_signal_fsm_process_event_left_hazard_left(void) {
  // State transitions: none -> left -> hazard-left -> hazard -> none

  // Go to left state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go to left-hazard state.
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);

  // Go to hazard state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
}

// Making sure left->right and right->left is possible.
void test_lights_signal_fsm_process_event_left_right(void) {
  // State transitions: none -> left -> right -> none -> right -> left -> none

  // Go to left state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  // Go to right state.
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);

  // Go back to none state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
  // Go to right state.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
  // Go to left state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go back to none state.
  const Event e6 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e6));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
}

// Making sure left-hazard->right-hazard and right-hazard -> left-hazard are possible.
void test_lights_signal_fsm_process_event_left_hazard_to_right_hazard(void) {
  // State transitions: none -> left -> left-hazard -> right-hazard -> left-hazard -> left -> none

  // Go to left state.
  const Event e1 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e1));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  // Go to left-hazard state.
  const Event e2 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e2));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);
  // Go to right-hazard state.
  const Event e3 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_RIGHT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e3));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // Go to left-hazard state.
  const Event e4 = { .id = LIGHTS_EVENT_SIGNAL_ON, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e4));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // Go to left state.
  const Event e5 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_HAZARD };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e5));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD);
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_ON,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);

  // Go back to none state.
  const Event e6 = { .id = LIGHTS_EVENT_SIGNAL_OFF, .data = LIGHTS_EVENT_SIGNAL_MODE_LEFT };
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_signal_fsm, &e6));
  TEST_LIGHTS_SIGNAL_ASSERT_RAISED_EVENT(LIGHTS_EVENT_GPIO_OFF,
                                         LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT);
}
