#include "power_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"

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
  return (e->id == INPUT_EVENT_POWER || e->id == INPUT_EVENT_MECHANICAL_BRAKE);
}

static bool prv_check_on(const Event *e) {
  return true;
}

// Power FSM output functions

static void prv_state_off(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_off;
}

static void prv_state_on(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_on;
}

void power_fsm_init(FSM *fsm) {
  fsm_state_init(state_off, prv_state_off);
  fsm_state_init(state_on, prv_state_on);

  void *context;

  event_arbiter_add_fsm(fsm, &context);

  fsm_init(fsm, "power_fsm", &state_off, context);
  prv_state_off(fsm, INPUT_EVENT_NONE, context);
}
