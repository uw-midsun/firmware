#include "direction_state.h"

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

// State machine transition tables

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

// Output functions for the direction state

static void prv_driver_state_neutral(FSM *fsm, const Event *e, void *context) {
  if (e->id == INPUT_EVENT_GAS_COAST || e->id == INPUT_EVENT_GAS_PRESSED) {
      *(bool*)context = 0;
      return;
  }
  *(bool*)context = 1;
}

static void prv_driver_state_forward(FSM *fsm, const Event *e, void *context) {
  if (e->id == INPUT_EVENT_POWER) {
    *(bool*)context = 0;
    return;
  }
  *(bool*)context = 1;
  return;
}

static void prv_driver_state_reverse(FSM *fsm, const Event *e, void *context) {
  if (e->id == INPUT_EVENT_POWER) {
    *(bool*)context = 0;
    return;
  }
  *(bool*)context = 1;
  return;
}

void direction_state_init(FSM *direction_fsm, void *context) {
	fsm_state_init(state_neutral, prv_driver_state_neutral);
	fsm_state_init(state_forward, prv_driver_state_forward);
	fsm_state_init(state_reverse, prv_driver_state_reverse);

	fsm_init(direction_fsm, "direction_fsm", &state_neutral, context);
}