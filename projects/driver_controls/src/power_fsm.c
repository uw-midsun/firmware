#include "power_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "log.h"

// Power FSM state definitions

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_on);

// Power FSM transition table definitions

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
}

FSM_STATE_TRANSITION(state_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
}

// Power FSM arbiter functions

static bool prv_check_off(const Event *e) {
  return (e->id == INPUT_EVENT_POWER ||
          e->id == INPUT_EVENT_MECHANICAL_BRAKE_RELEASED ||
          e->id == INPUT_EVENT_MECHANICAL_BRAKE_PRESSED);
}

static bool prv_check_on(const Event *e) {
  return true;
}

// Power FSM output functions

static void prv_state_off(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_off;

  InputEventData *data = &e->data;
  data->components.state = 0;

  event_raise(INPUT_EVENT_CAN_ID_POWER, e->data);
}

static void prv_state_on(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_on;

  InputEventData *data = &e->data;
  data->components.state = 1;

  event_raise(INPUT_EVENT_CAN_ID_POWER, e->data);
}

StatusCode power_fsm_init(FSM *fsm) {
  fsm_state_init(state_off, prv_state_off);
  fsm_state_init(state_on, prv_state_on);

  fsm_init(fsm, "power_fsm", &state_off, NULL);

  fsm->context = event_arbiter_add_fsm(fsm, prv_check_off);

  if (fsm->context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  return STATUS_CODE_OK;
}
