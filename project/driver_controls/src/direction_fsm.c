#include "direction_fsm.h"
#include "input_event.h"

// Direction selector FSM state definitions

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

// Direction selector FSM transition table definitions

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

// Direction selector FSM arbiter functions

static bool prv_check_neutral(const Event *e) {
  return !(e->id == INPUT_EVENT_GAS_COAST || e->id == INPUT_EVENT_GAS_PRESSED);
}

static bool prv_check_driver(const Event *e) {
  return (e->id != INPUT_EVENT_POWER);
}

// Direction selector FSM output functions

static void prv_state_neutral(FSM *fsm, const Event *e, void *context) {
  InputEventCheck *event_check = fsm->context;
  *event_check = prv_check_neutral;
}

static void prv_state_drive(FSM *fsm, const Event *e, void *context) {
  InputEventCheck *event_check = fsm->context;
  *event_check = prv_check_driver;
}

void direction_fsm_init(FSM *direction_fsm, void *context) {
  fsm_state_init(state_neutral, prv_state_neutral);
  fsm_state_init(state_forward, prv_state_drive);
  fsm_state_init(state_reverse, prv_state_drive);

  fsm_init(direction_fsm, "direction_fsm", &state_neutral, context);
  prv_state_neutral(direction_fsm, INPUT_EVENT_NONE, direction_fsm->context);
}
