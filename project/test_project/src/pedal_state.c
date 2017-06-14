#include "pedal_state.h"
#include <stdio.h>

FSM_DECLARE_STATE(state_off);             // Off State: Car is not receiving power
FSM_DECLARE_STATE(state_brake);           // Brake State: Driver is holding down the brake pedal
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);         // Driving State: Car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_cruise_control);  // Driving State: Car is in motion due to the gas pedal

// Transition guard functions

static bool prv_power_guard(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  bool transitioned = (fsm_group->direction.state == STATE_NEUTRAL);
  return transitioned;
}

static bool prv_gas_guard(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  bool transitioned = (fsm_group->direction.state == STATE_FORWARD) ||
                      (fsm_group->direction.state == STATE_REVERSE);
  return transitioned;
}

// State machine transition tables

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_ON, state_brake);
}

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_POWER_OFF, prv_power_guard, state_off);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_GAS_COAST, prv_gas_guard, state_coast);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_GAS_PRESSED, prv_gas_guard, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_GAS_PRESSED, prv_gas_guard, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_ON, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_GAS_COAST, prv_gas_guard, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_ON, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_OFF, state_brake);
}

// Output functions for the pedal state

static void prv_driver_state_off(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->pedal.state = STATE_OFF;
}

static void prv_driver_state_brake(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->pedal.state = STATE_BRAKE;
}

static void prv_driver_state_coast(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->pedal.state = STATE_COAST;
}

static void prv_driver_state_driving(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->pedal.state = STATE_DRIVING;
}

static void prv_driver_state_cruise_control(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->pedal.state = STATE_CRUISE_CONTROL;
}

void pedal_state_init(FSM* pedal_fsm, FSMGroup* fsm_group) {
	fsm_state_init(state_off, prv_driver_state_off);
	fsm_state_init(state_brake, prv_driver_state_brake);
	fsm_state_init(state_coast, prv_driver_state_coast);
	fsm_state_init(state_driving, prv_driver_state_driving);
	fsm_state_init(state_cruise_control, prv_driver_state_cruise_control);

	fsm_init(pedal_fsm, "pedal_fsm", &state_off, fsm_group);
}