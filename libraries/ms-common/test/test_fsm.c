#include "fsm.h"
#include "unity.h"

typedef enum {
  TEST_FSM_EVENT_A = 0,
  TEST_FSM_EVENT_B,
  TEST_FSM_EVENT_C,
} TEST_FSM_EVENT;

static FSM s_fsm;
static bool s_output;

FSM_DECLARE_STATE(test_a);
FSM_DECLARE_STATE(test_b);
FSM_DECLARE_STATE(test_c);

FSM_STATE_TRANSITION(test_a) {
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_A, test_a);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_B, test_b);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_C, test_c);
}

FSM_STATE_TRANSITION(test_b) {
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_C, test_c);
  FSM_ADD_TRANSITION(TEST_FSM_EVENT_A, test_a);
}

FSM_STATE_TRANSITION(test_c) {

}

static void prv_final_state(struct FSM *fsm, const Event *e) {
  printf("[%s:%s] Final state reached from %s (Event %d, data %d)\n",
         fsm->name, fsm->current_state->name, fsm->last_state->name,
         e->id, e->data);
  s_output = true;
}

void setup_test(void) {
  fsm_state_init(test_c, prv_final_state);
  fsm_init(&s_fsm, "test_fsm", &test_a);
  s_output = false;
}

void teardown_test(void) { }

void test_fsm_transition(void) {
  Event e = {
    .id = TEST_FSM_EVENT_A,
    .data = 10
  };

  // Expect A -> A -> B -> C, fail to transition
  bool transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);

  e.id = TEST_FSM_EVENT_B;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);

  e.id = TEST_FSM_EVENT_C;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_TRUE(transitioned);
  TEST_ASSERT_TRUE(s_output);

  e.id = TEST_FSM_EVENT_A;
  transitioned = fsm_process_event(&s_fsm, &e);
  TEST_ASSERT_FALSE(transitioned);
}
