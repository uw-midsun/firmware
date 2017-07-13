#include "pedal_state.h"
#include "input_event.h"
#include <stdio.h>

FSM_DECLARE_STATE(state_brake);           // Brake State: Driver is holding down the brake pedal
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);         // Driving State: Car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_cruise_control);  // Driving State: Car is in motion due to the gas pedal

// State machine transition tables

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

// State output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  bool *permitted = fsm->context;
  *permitted = true;
}

void pedal_state_init(FSM *pedal_fsm, void *context) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_state_output);

  fsm_init(pedal_fsm, "pedal_fsm", &state_brake, context);
}
