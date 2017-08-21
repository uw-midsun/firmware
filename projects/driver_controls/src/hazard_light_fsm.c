#include "hazard_light_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "log.h"

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

// Hazard light FSM output function

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  InputEventData data;
  data.components.data = e->data;

  if (fsm->current_state == &state_hazard_on) {
    data.components.state = HAZARD_LIGHT_FSM_STATE_ON;
  } else if (fsm->current_state == &state_hazard_off) {
    data.components.state = HAZARD_LIGHT_FSM_STATE_OFF;
  }

  data.components.digital = true;

  event_raise(INPUT_EVENT_CAN_ID_HAZARD_LIGHT, data.raw);
}

StatusCode hazard_light_fsm_init(FSM *fsm) {
  fsm_state_init(state_hazard_on, prv_state_output);
  fsm_state_init(state_hazard_off, prv_state_output);

  void *context = event_arbiter_add_fsm(fsm, NULL);

  if (context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "hazard_light_fsm", &state_hazard_off, context);

  return STATUS_CODE_OK;
}
