#include "turn_signal_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"

// Turn signal FSM state definitions

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

// Turn signal FSM transition table definitions
// TODO: Power off event turns signals off
FSM_STATE_TRANSITION(state_no_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_LEFT, state_left_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_NONE, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_right_signal) {
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
}

void turn_signal_fsm_init(FSM* turn_signal_fsm, void *context) {
  fsm_state_init(state_no_signal, prv_state_output);
  fsm_state_init(state_left_signal, prv_state_output);
  fsm_state_init(state_right_signal, prv_state_output);

  fsm_init(turn_signal_fsm, "turn_signal_fsm", &state_no_signal, context);
  prv_state_output(turn_signal_fsm, INPUT_EVENT_NONE, turn_signal_fsm->context);
}
