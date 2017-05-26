#pragma once
#include "fsm.h"
#include "event_queue.h"

// Define output functions for the Driver control FSM

void driver_state_off(FSM* fsm, const Event* e, void *context);

void driver_state_idle_neutral(FSM* fsm, const Event* e, void *context);
void driver_state_idle_forward(FSM* fsm, const Event* e, void *context);
void driver_state_idle_backward(FSM* fsm, const Event* e, void *context);

void driver_state_driving_forward(FSM* fsm, const Event* e, void *context);
void driver_state_driving_backward(FSM* fsm, const Event* e, void *context);

void driver_state_brake_neutral(FSM* fsm, const Event* e, void *context);
void driver_state_brake_forward(FSM* fsm, const Event* e, void *context);
void driver_state_brake_backward(FSM* fsm, const Event* e, void *context);

typedef enum {
  INPUT_EVENT_POWER_ON,
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
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_HAZARD_LIGHT_ON,
  INPUT_EVENT_HAZARD_LIGHT_OFF,
  INPUT_EVENT_CRUISE_CONTROL_ON,
  INPUT_EVENT_CRUISE_CONTROL_OFF,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON
} InputEvent;

// Off State: when the car is not receiving power
FSM_DECLARE_STATE(state_off);

// Idle State: when neither of the gas pedals are pressed
FSM_DECLARE_STATE(state_idle_neutral);
FSM_DECLARE_STATE(state_idle_forward);
FSM_DECLARE_STATE(state_idle_reverse);

// Brake State: when the driver is holding down the brake pedal
FSM_DECLARE_STATE(state_brake_neutral);
FSM_DECLARE_STATE(state_brake_forward);
FSM_DECLARE_STATE(state_brake_reverse);

// Driving State: when the car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_driving_forward);
FSM_DECLARE_STATE(state_driving_reverse);

// State table for off state
FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_ON, state_idle_neutral);
}

// State table for idle superstate
FSM_STATE_TRANSITION(state_idle_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_OFF, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_idle_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_idle_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

// State table for brake superstate
FSM_STATE_TRANSITION(state_brake_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_brake_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_brake_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

// State table for driving superstate
FSM_STATE_TRANSITION(state_driving_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_RELEASED, state_idle_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_driving_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_RELEASED, state_idle_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}