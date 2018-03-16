#include "event_arbiter.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"
#include <stddef.h>

typedef enum {
  TEST_EVENT_ARBITER_EVENT_A = 0,
  TEST_EVENT_ARBITER_EVENT_B,
  TEST_EVENT_ARBITER_EVENT_C,
  NUM_TEST_EVENT_ARBITER_EVENTS
} TestEventArbiterEvent;

FSM_DECLARE_STATE(state_a);
FSM_DECLARE_STATE(state_b);
FSM_DECLARE_STATE(state_c);

FSM_STATE_TRANSITION(state_a) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_A, state_a);
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_B, state_b);
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_C, state_c);
}

FSM_STATE_TRANSITION(state_b) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_A, state_a);
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_B, state_b);
}

FSM_STATE_TRANSITION(state_c) {
  // Empty state to keep FSM B constant
}

static FSM s_fsm_a;
static FSM s_fsm_b;
static EventArbiterStorage s_arbiter_storage;

static uint8_t s_output_runs;

static bool prv_disable_event_b(const Event *e) {
  return (e->id != TEST_EVENT_ARBITER_EVENT_B);
}

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  s_output_runs++;
}

void setup_test(void) {
  s_output_runs = 0;

  event_arbiter_init(&s_arbiter_storage);

  fsm_state_init(state_a, prv_state_output);
  fsm_state_init(state_b, prv_state_output);
  fsm_state_init(state_c, prv_state_output);
}

void teardown_test(void) {}

void test_event_arbiter_add(void) {
  for (uint8_t i = 0; i < EVENT_ARBITER_MAX_FSMS; i++) {
    TEST_ASSERT_NOT_EQUAL(NULL, event_arbiter_add_fsm(&s_arbiter_storage, &s_fsm_a, NULL));
  }
  TEST_ASSERT_EQUAL(NULL, event_arbiter_add_fsm(&s_arbiter_storage, &s_fsm_a, NULL));
}

void test_event_arbiter_basic(void) {
  Event e = { 0 };

  // Make sure the default arbiter allows all messages through
  EventArbiter *arbiter_a = event_arbiter_add_fsm(&s_arbiter_storage, &s_fsm_a, NULL);
  fsm_init(&s_fsm_a, "FSM A", &state_a, arbiter_a);

  EventArbiter *arbiter_b = event_arbiter_add_fsm(&s_arbiter_storage, &s_fsm_b, NULL);
  fsm_init(&s_fsm_b, "FSM B", &state_c, arbiter_b);

  // Test that the context pointers point to the stored arbiter functions
  TEST_ASSERT_NOT_EQUAL(NULL, s_fsm_a.context);
  TEST_ASSERT_NOT_EQUAL(NULL, s_fsm_b.context);

  // Move FSM A to state B
  e.id = TEST_EVENT_ARBITER_EVENT_B;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&s_arbiter_storage, &e));
  TEST_ASSERT_EQUAL(1, s_output_runs);

  // Move FSM A to state A
  e.id = TEST_EVENT_ARBITER_EVENT_A;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&s_arbiter_storage, &e));
  TEST_ASSERT_EQUAL(2, s_output_runs);

  // Switch FSM B's arbiter to disable event B
  event_arbiter_set_event_check(arbiter_b, prv_disable_event_b);

  // Fail to move FSM A to state B
  e.id = TEST_EVENT_ARBITER_EVENT_B;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&s_arbiter_storage, &e));
  TEST_ASSERT_EQUAL(2, s_output_runs);

  // Move FSM A to state C
  e.id = TEST_EVENT_ARBITER_EVENT_C;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&s_arbiter_storage, &e));
  TEST_ASSERT_EQUAL(3, s_output_runs);
}
