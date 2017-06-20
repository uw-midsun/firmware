#include "turn_signal_state.h"

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

// Transition guard function
static bool prv_power_guard(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  bool transitioned = (fsm_group->pedal.state != STATE_OFF);
  return transitioned;
}

// State machine transition tables

FSM_STATE_TRANSITION(state_no_signal) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_LEFT, prv_power_guard, state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, prv_power_guard, state_right_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_NONE, prv_power_guard, state_no_signal);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_RIGHT, prv_power_guard, state_right_signal);
}

FSM_STATE_TRANSITION(state_right_signal) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_LEFT, prv_power_guard, state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_TURN_SIGNAL_NONE, prv_power_guard, state_no_signal);
}

// Output functions for the turn signal state

static void prv_driver_state_no_signal(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->turn_signal.state = STATE_NO_SIGNAL;
}

static void prv_driver_state_left_signal(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->turn_signal.state = STATE_LEFT_SIGNAL;
}

static void prv_driver_state_right_signal(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->turn_signal.state = STATE_RIGHT_SIGNAL;
}


void turn_signal_state_init(FSM* turn_signal_fsm, FSMGroup* fsm_group) {
	fsm_state_init(state_no_signal, prv_driver_state_no_signal);
	fsm_state_init(state_left_signal, prv_driver_state_left_signal);
	fsm_state_init(state_right_signal, prv_driver_state_right_signal);

	fsm_init(turn_signal_fsm, "turn_signal_fsm", &state_no_signal, fsm_group);
}