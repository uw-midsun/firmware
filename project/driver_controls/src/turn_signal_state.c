#include "turn_signal_state.h"
#include "input_event.h"

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

// State machine transition tables

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

// State output functions
static void prv_state_output(FSM* fsm, const Event* e, void *context) {
  bool *permitted = fsm->context;
  *permitted = true;
}

void turn_signal_state_init(FSM* turn_signal_fsm, void *context) {
  fsm_state_init(state_no_signal, prv_state_output);
  fsm_state_init(state_left_signal, prv_state_output);
  fsm_state_init(state_right_signal, prv_state_output);

  fsm_init(turn_signal_fsm, "turn_signal_fsm", &state_no_signal, context);
}
