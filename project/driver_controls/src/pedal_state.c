#include "pedal_state.h"
#include <stdio.h>

FSM_DECLARE_STATE(state_off);             // Off State: Car is not receiving power
FSM_DECLARE_STATE(state_brake);           // Brake State: Driver is holding down the brake pedal
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);         // Driving State: Car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_cruise_control);  // Driving State: Car is in motion due to the gas pedal

// Transition guard functions
static bool prv_check_event(const Event *e) {
  switch (e->id) {
    case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:
    case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:
    case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
      return false;
    default:
      return true;
  }
  return;
}

// State machine transition tables

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_brake);
}

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_brake);
}

// Output functions for the pedal state

static void prv_driver_state_off(FSM *fsm, const Event *e, void *context) {
  if (e->id == INPUT_EVENT_POWER) {
    *(bool*)context = 1;
    return;
  }

  *(bool*)context = 0;
}

static void prv_driver_state_brake(FSM *fsm, const Event *e, void *context) {
  *(bool*)context = 1;
}

static void prv_driver_state_coast(FSM *fsm, const Event *e, void *context) {
  *(bool*)context = prv_check_event(e);
}

static void prv_driver_state_driving(FSM *fsm, const Event *e, void *context) {
  *(bool*)context = prv_check_event(e);
}

static void prv_driver_state_cruise_control(FSM *fsm, const Event *e, void *context) {
  *(bool*)context = prv_check_event(e);
}

void pedal_state_init(FSM *pedal_fsm, void *context) {
	fsm_state_init(state_off, prv_driver_state_off);
	fsm_state_init(state_brake, prv_driver_state_brake);
	fsm_state_init(state_coast, prv_driver_state_coast);
	fsm_state_init(state_driving, prv_driver_state_driving);
	fsm_state_init(state_cruise_control, prv_driver_state_cruise_control);

	fsm_init(pedal_fsm, "pedal_fsm", &state_off, context);
}