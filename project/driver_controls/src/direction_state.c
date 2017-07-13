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

// State output functions
static void prv_state_neutral(FSM *fsm, const Event *e, void *context) {
  bool *permitted = fsm->context;
  *permitted = !(e->id == INPUT_EVENT_GAS_COAST || e->id == INPUT_EVENT_GAS_PRESSED);
}

static void prv_state_forward(FSM *fsm, const Event *e, void *context) {
  bool *permitted = fsm->context;
  *permitted = (e->id != INPUT_EVENT_POWER);
}

void direction_state_init(FSM *direction_fsm, void *context) {
  fsm_state_init(state_neutral, prv_state_neutral);
  fsm_state_init(state_forward, prv_state_forward);
  fsm_state_init(state_reverse, prv_state_forward);

  fsm_init(direction_fsm, "direction_fsm", &state_neutral, context);
}
