#include "driver_state.h"
#include <stdio.h>

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

FSM_DECLARE_STATE(state_no_signal);
FSM_DECLARE_STATE(state_left_signal);
FSM_DECLARE_STATE(state_right_signal);

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// Transition table for direction state machine

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

// Transition table for the turn signal state machine

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

// Transition table for the turn signal state machine

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT_OFF, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT_ON, state_hazard_on);
}

static void prv_driver_state_neutral(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_NEUTRAL;
}

static void prv_driver_state_forward(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_FORWARD;
}

static void prv_driver_state_reverse(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_REVERSE;
}

static void prv_driver_state_no_signal(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_NO_SIGNAL;
}

static void prv_driver_state_left_signal(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_LEFT_SIGNAL;
}

static void prv_driver_state_right_signal(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_RIGHT_SIGNAL;
}

static void prv_driver_state_hazard_on(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_HAZARD_ON;
}

static void prv_driver_state_hazard_off(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_HAZARD_OFF;
}

void state_init(FSMGroup* fsm_group, FSMState* default_state) {
	fsm_state_init(state_neutral, prv_driver_state_neutral);
	fsm_state_init(state_forward, prv_driver_state_forward);
	fsm_state_init(state_reverse, prv_driver_state_reverse);

	fsm_state_init(state_no_signal, prv_driver_state_no_signal);
	fsm_state_init(state_left_signal, prv_driver_state_left_signal);
	fsm_state_init(state_right_signal, prv_driver_state_right_signal);

	fsm_state_init(state_hazard_on, prv_driver_state_hazard_on);
	fsm_state_init(state_hazard_off, prv_driver_state_hazard_off);

  pedal_state_init(&fsm_group->pedal_fsm, &default_state[0]);
  printf("address = %#x, data = %d\n",
          (uint8_t*)fsm_group->pedal_fsm.context,
          *(uint8_t*)fsm_group->pedal_fsm.context);
  
  fsm_init(&fsm_group->direction_fsm, "direction_fsm", &state_neutral, &default_state[1]);
  fsm_init(&fsm_group->turn_signal_fsm, "turn_signal_fsm", &state_no_signal, &default_state[2]);
  fsm_init(&fsm_group->hazard_light_fsm, "hazard_light_fsm", &state_hazard_off, &default_state[3]);
}

void state_process_event(FSMGroup* fsm_group, Event* e) {
  if (e->id <= 8) {
    fsm_process_event(&fsm_group->pedal_fsm, e); 
  } else if (e->id <= 11) {
    fsm_process_event(&fsm_group->direction_fsm, e);
  } else if (e->id <= 14) {
    fsm_process_event(&fsm_group->turn_signal_fsm, e);
  } else if (e->id <= 16) {
    fsm_process_event(&fsm_group->hazard_light_fsm, e);
  }
}