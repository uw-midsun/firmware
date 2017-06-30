#include "pedal_state.h"
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

// Transition check functions
static bool prv_check_brake(const Event *e) {
  return true;
}

static bool prv_check_event(const Event *e) {
  switch (e->id) {
    case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:
    case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:
    case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
      return false;
    default:
      return true;
  }
}

// State output functions
static void prv_state_brake(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_brake;
}

static void prv_state_coast(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_event;
}

static void prv_state_driving(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_event;
}

static void prv_state_cruise_control(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_event;
}

void pedal_state_init(FSM *pedal_fsm, void *context) {
	fsm_state_init(state_brake, prv_state_brake);
	fsm_state_init(state_coast, prv_state_coast);
	fsm_state_init(state_driving, prv_state_driving);
	fsm_state_init(state_cruise_control, prv_state_cruise_control);

	fsm_init(pedal_fsm, "pedal_fsm", &state_brake, prv_check_brake);
}