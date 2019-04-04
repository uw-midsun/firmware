#include "turn_signal_fsm.h"
#include "can_transmit.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "input_event.h"

// Turn signal FSM state definitions
FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

// Turn signal FSM transition table definitions
FSM_STATE_TRANSITION(state_no_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, state_left_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_no_signal);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT, state_right_signal);
}

FSM_STATE_TRANSITION(state_right_signal) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_no_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_no_signal);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT, state_left_signal);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE, state_no_signal);
}

// Turn signal FSM output function
static void prv_no_signal_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_LEFT, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_RIGHT, EE_LIGHT_STATE_OFF);
}

static void prv_left_signal_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_LEFT, EE_LIGHT_STATE_ON);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_RIGHT, EE_LIGHT_STATE_OFF);
}

static void prv_right_signal_output(Fsm *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_LEFT, EE_LIGHT_STATE_OFF);
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_RIGHT, EE_LIGHT_STATE_ON);
}

StatusCode turn_signal_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_no_signal, prv_no_signal_output);
  fsm_state_init(state_left_signal, prv_left_signal_output);
  fsm_state_init(state_right_signal, prv_right_signal_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Turn Signal FSM", &state_no_signal, guard);

  return STATUS_CODE_OK;
}
