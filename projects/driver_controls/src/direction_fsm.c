#include "direction_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "log.h"

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
  return !(e->id == INPUT_EVENT_PEDAL_COAST || e->id == INPUT_EVENT_PEDAL_PRESSED);
}

static bool prv_check_forward(const Event *e) {
  return (e->id != INPUT_EVENT_POWER);
}

static bool prv_check_reverse(const Event *e) {
  return !(e->id == INPUT_EVENT_POWER || e->id == INPUT_EVENT_CRUISE_CONTROL);
}

// Direction selector FSM output functions

static void prv_state_neutral(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_neutral;

  uint8_t state = 0;
  LOG_DEBUG("Direction Selector State = %d\n", state);
}

static void prv_state_forward(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_forward;

  uint8_t state = 1;
  LOG_DEBUG("Direction Selector State = %d\n", state);
}

static void prv_state_reverse(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_reverse;

  uint8_t state = 2;
  LOG_DEBUG("Direction Selector State = %d\n", state);
}

StatusCode direction_fsm_init(FSM *fsm) {
  fsm_state_init(state_neutral, prv_state_neutral);
  fsm_state_init(state_forward, prv_state_forward);
  fsm_state_init(state_reverse, prv_state_reverse);

  fsm_init(fsm, "direction_fsm", &state_neutral, NULL);

  fsm->context = event_arbiter_add_fsm(fsm, prv_check_neutral);

  if (fsm->context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  return STATUS_CODE_OK;
}
