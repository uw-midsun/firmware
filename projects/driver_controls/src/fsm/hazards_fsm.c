#include "hazards_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "exported_enums.h"
#include "can_transmit.h"

// Hazard light FSM state definitions
FSM_DECLARE_STATE(state_hazard_off);
FSM_DECLARE_STATE(state_hazard_on);

// Hazard light FSM transition table definitions
FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_on);
}

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_hazard_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_hazard_off);
}

// Hazard light FSM output function
static void prv_hazard_off_output(FSM *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_ON);
}

static void prv_hazard_on_output(FSM *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_OFF);
}

StatusCode hazard_light_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_hazard_on, prv_state_output);
  fsm_state_init(state_hazard_off, prv_state_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Hazard Light FSM", &state_hazard_off, guard);

  return STATUS_CODE_OK;
}
