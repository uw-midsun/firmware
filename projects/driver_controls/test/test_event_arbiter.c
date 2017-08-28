#include "event_arbiter.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_EVENT_ARBITER_EVENT_A,
  TEST_EVENT_ARBITER_EVENT_B,
  TEST_EVENT_ARBITER_EVENT_C,
  TEST_EVENT_ARBITER_EVENT_D
} TEST_EVENT_ARBITER_EVENT;

FSM_DECLARE_STATE(state_a);
FSM_DECLARE_STATE(state_b);
FSM_DECLARE_STATE(state_c);
FSM_DECLARE_STATE(state_d);

FSM_STATE_TRANSITION(state_a) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_B, state_b);
}

FSM_STATE_TRANSITION(state_b) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_A, state_a);
}

FSM_STATE_TRANSITION(state_c) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_D, state_d);
}

FSM_STATE_TRANSITION(state_d) {
  FSM_ADD_TRANSITION(TEST_EVENT_ARBITER_EVENT_C, state_c);
}

static FSM s_fsm_a;
static FSM s_fsm_b;

static bool prv_check_state_c(const Event *e) {
  return (e->id != TEST_EVENT_ARBITER_EVENT_B);
}

static void prv_state_c(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_state_c;
}

static void prv_state_d(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = NULL;
}

void setup_test() {
  fsm_state_init(state_c, prv_state_c);
  fsm_state_init(state_d, prv_state_d);

  fsm_init(&s_fsm_a, "test_fsm_a", &state_a, NULL);
  fsm_init(&s_fsm_b, "test_fsm_b", &state_c, prv_check_state_c);

  event_arbiter_init(NULL);
}

void teardown_test() {}

void test_event_arbiter_add() {
  for (uint8_t i = 0; i < EVENT_ARBITER_MAX_FSMS; i++) {
    TEST_ASSERT_NOT_EQUAL(NULL, event_arbiter_add_fsm(&s_fsm_a, NULL));
  }
  TEST_ASSERT_EQUAL(NULL, event_arbiter_add_fsm(&s_fsm_a, NULL));
}

void test_event_arbiter_process() {
  Event e;

  s_fsm_a.context = event_arbiter_add_fsm(&s_fsm_a, NULL);
  s_fsm_b.context = event_arbiter_add_fsm(&s_fsm_b, prv_check_state_c);

  // Test that the context pointers point to the stored arbiter functions
  TEST_ASSERT_NOT_EQUAL(NULL, s_fsm_a.context);
  TEST_ASSERT_NOT_EQUAL(NULL, s_fsm_b.context);

  e.id = TEST_EVENT_ARBITER_EVENT_B;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = TEST_EVENT_ARBITER_EVENT_D;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = TEST_EVENT_ARBITER_EVENT_B;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}
