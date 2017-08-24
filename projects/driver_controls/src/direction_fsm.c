#include "direction_fsm.h"
#include "can_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
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
  // The car must not be able to move while in the neutral state
  switch (e->id) {
    case INPUT_EVENT_PEDAL_COAST:
    case INPUT_EVENT_PEDAL_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
      return false;
    default:
      return true;
  }
}

static bool prv_check_forward(const Event *e) {
  // Powering off while the car is moving can be dangerous. As a result, powering off is
  // forbidden while in forward gear
  return (e->id != INPUT_EVENT_POWER);
}

static bool prv_check_reverse(const Event *e) {
  // Same as forward, except that cruise control is forbidden in reverse for obvious reasons
  return !(e->id == INPUT_EVENT_POWER || e->id == INPUT_EVENT_CRUISE_CONTROL);
}

// Direction selector FSM output functions

static void prv_state_neutral(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_neutral;

  can_fsm_transmit(CAN_DEVICE_ID_DIRECTION_SELECTOR, DIRECTION_FSM_STATE_NEUTRAL, e->data);
}

static void prv_state_forward(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_forward;

  can_fsm_transmit(CAN_DEVICE_ID_DIRECTION_SELECTOR, DIRECTION_FSM_STATE_FORWARD, e->data);
}

static void prv_state_reverse(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_reverse;

  can_fsm_transmit(CAN_DEVICE_ID_DIRECTION_SELECTOR, DIRECTION_FSM_STATE_REVERSE, e->data);
}

StatusCode direction_fsm_init(FSM *fsm) {
  fsm_state_init(state_neutral, prv_state_neutral);
  fsm_state_init(state_forward, prv_state_forward);
  fsm_state_init(state_reverse, prv_state_reverse);

  void *context = event_arbiter_add_fsm(fsm, prv_check_neutral);

  if (context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "direction_fsm", &state_neutral, context);

  return STATUS_CODE_OK;
}
