#include "turn_signal_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "log.h"

// Turn signal FSM state definitions

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

// Turn signal FSM transition table definitions

FSM_STATE_TRANSITION(state_no_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_LEFT, state_left_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_NONE, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_right_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_LEFT, state_left_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_NONE, state_no_signal);
}

// Turn signal FSM arbiter function

static bool prv_check_turn_signal(const Event *e) {
  return true;
}

// Turn signal FSM output function

static void prv_state_output(FSM* fsm, const Event* e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_turn_signal;

  uint8_t state = (fsm->current_state->table == state_left_signal->table) |
                  ((fsm->current_state->table == state_right_signal->table) << 1);

  LOG_DEBUG("Turn Signal State = %d\n", state);
}

StatusCode turn_signal_fsm_init(FSM* fsm) {
  fsm_state_init(state_no_signal, prv_state_output);
  fsm_state_init(state_left_signal, prv_state_output);
  fsm_state_init(state_right_signal, prv_state_output);

  fsm_init(fsm, "turn_signal_fsm", &state_no_signal, NULL);

  fsm->context = event_arbiter_add_fsm(fsm, prv_check_turn_signal);

  if (fsm->context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  return STATUS_CODE_OK;
}
