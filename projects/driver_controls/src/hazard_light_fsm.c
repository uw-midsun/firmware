#include "hazard_light_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"

// Hazard light FSM state definitions

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// Hazard light FSM transition table definitions

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_on);
}

// Hazard light FSM arbiter function

static bool prv_check_hazard_light(const Event *e) {
  return true;
}

// Hazard light FSM output function

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_hazard_light;
}

StatusCode hazard_light_fsm_init(FSM *fsm) {
  fsm_state_init(state_hazard_on, prv_state_output);
  fsm_state_init(state_hazard_off, prv_state_output);

  fsm_init(fsm, "hazard_light_fsm", &state_hazard_off, NULL);

  if (!status_ok(event_arbiter_add_fsm(fsm, prv_check_hazard_light))) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }

  return STATUS_CODE_OK;
}
