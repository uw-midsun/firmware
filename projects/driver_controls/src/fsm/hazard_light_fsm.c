#include "hazard_light_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
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
  HazardLightFSMState hazard_light_state = HAZARD_LIGHT_FSM_STATE_OFF;

  if (fsm->current_state == &state_hazard_on) {
    hazard_light_state = HAZARD_LIGHT_FSM_STATE_ON;
  }

  (void)hazard_light_state;
  // Previous: Output Hazard state
}

StatusCode hazard_light_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_hazard_on, prv_state_output);
  fsm_state_init(state_hazard_off, prv_state_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "hazard_light_fsm", &state_hazard_off, guard);

  return STATUS_CODE_OK;
}
