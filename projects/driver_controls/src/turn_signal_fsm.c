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

// Turn signal FSM output function

static void prv_state_output(FSM* fsm, const Event* e, void *context) {
  InputEventData *data = &e->data;

  // Use bitwise operation to determine state
  // 0 = state_no_signal
  // 1 = state_left_signal
  // 2 = state_right_signal
  data->components.state = (fsm->current_state == &state_left_signal) |
                          ((fsm->current_state == &state_right_signal) << 1);

  event_raise(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e->data);
}

StatusCode turn_signal_fsm_init(FSM* fsm) {
  fsm_state_init(state_no_signal, prv_state_output);
  fsm_state_init(state_left_signal, prv_state_output);
  fsm_state_init(state_right_signal, prv_state_output);

  fsm_init(fsm, "turn_signal_fsm", &state_no_signal, NULL);

  fsm->context = event_arbiter_add_fsm(fsm, NULL);

  if (fsm->context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  return STATUS_CODE_OK;
}
