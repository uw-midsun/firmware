#pragma once
#include "fsm.h"
#include "event_queue.h"

// Define output functions for the Driver control FSM

void driver_state_off(FSM* fsm, const Event* e, void *context);

void driver_state_idle(FSM* fsm, const Event* e, void *context);
void driver_state_brake(FSM* fsm, const Event* e, void *context);
void driver_state_driving(FSM* fsm, const Event* e, void *context);

void driver_state_neutral(FSM* fsm, const Event* e, void *context);
void driver_state_forward(FSM* fsm, const Event* e, void *context);
void driver_state_reverse(FSM* fsm, const Event* e, void *context);

typedef enum {
  INPUT_EVENT_POWER_ON = 0,
  INPUT_EVENT_POWER_OFF,
  INPUT_EVENT_GAS_PRESSED,
  INPUT_EVENT_GAS_RELEASED,
  INPUT_EVENT_BRAKE_PRESSED,
  INPUT_EVENT_BRAKE_RELEASED,
  INPUT_EVENT_HORN_PRESSED,
  INPUT_EVENT_HORN_RELEASED,
  INPUT_EVENT_EMERGENCY_STOP,
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,
  INPUT_EVENT_HEADLIGHTS_ON,
  INPUT_EVENT_HEADLIGHTS_OFF,
  INPUT_EVENT_TURN_SIGNAL_NONE,
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_HAZARD_LIGHT_ON,
  INPUT_EVENT_HAZARD_LIGHT_OFF,
  INPUT_EVENT_CRUISE_CONTROL_ON,
  INPUT_EVENT_CRUISE_CONTROL_OFF,
  INPUT_EVENT_CRUISE_CONTROL_INC,
  INPUT_EVENT_CRUISE_CONTROL_DEC,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON
} InputEvent;

typedef struct FSMGroup {
    FSM pedal_fsm;
    FSM direction_fsm;
    FSM turn_signal_fsm;
} FSMGroup;

FSM_DECLARE_STATE(state_off);             // Off State: Car is not receiving power
FSM_DECLARE_STATE(state_idle);            // Idle State: Neither of the gas pedals are pressed
FSM_DECLARE_STATE(state_brake);           // Brake State: Driver is holding down the brake pedal
FSM_DECLARE_STATE(state_driving);         // Driving State: Car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_cruise_control);  // Driving State: Car is in motion due to the gas pedal

FSM_DECLARE_STATE(state_neutral); 
FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

/********************  Transition table for pedal state machine  ******************/
FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_ON, state_idle);
}

FSM_STATE_TRANSITION(state_idle) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_OFF, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_ON, state_cruise_control);
}

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_RELEASED, state_idle);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_ON, state_cruise_control);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_OFF, state_idle);
}

/********************  Transition table for direction state machine  ******************/
FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_neutral);
}

/********************  Transition table for the turn signal state machine  ******************/
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