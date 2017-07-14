#include "pedal_fsm.h"
#include "input_event.h"

// Pedal FSM state definitions

FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);
FSM_DECLARE_STATE(state_cruise_control);

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_brake);
}

// Pedal FSM arbiter function

static bool prv_check_pedal(Event *e) {
  return true;
}

// Pedal FSM output functions

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  InputEventCheck *event_check = fsm->context;
  *event_check = prv_check_pedal;
}

void pedal_state_init(FSM *pedal_fsm, void *context) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_state_output);

  fsm_init(pedal_fsm, "pedal_fsm", &state_brake, context);
  prv_state_output(pedal_fsm, INPUT_EVENT_NONE, pedal_fsm->context);
}
