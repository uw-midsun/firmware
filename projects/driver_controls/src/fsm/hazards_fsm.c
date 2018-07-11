#include "hazards_fsm.h"
#include "can_transmit.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "input_event.h"
#include "log.h"

// Hazard light FSM state definitions
FSM_DECLARE_STATE(state_hazard_off);
FSM_DECLARE_STATE(state_hazard_on);

// Hazard light FSM transition table definitions
FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED, state_hazard_on);
}

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED, state_hazard_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_hazard_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_hazard_off);
}

// Hazard light FSM output function
static void prv_hazard_off_output(FSM *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_OFF);
  event_raise(INPUT_EVENT_HAZARDS_STATE_OFF, 0);
}

static void prv_hazard_on_output(FSM *fsm, const Event *e, void *context) {
  CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_SIGNAL_HAZARD, EE_LIGHT_STATE_ON);
  event_raise(INPUT_EVENT_HAZARDS_STATE_ON, 0);
}

StatusCode hazards_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_hazard_on, prv_hazard_on_output);
  fsm_state_init(state_hazard_off, prv_hazard_off_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Hazard Light FSM", &state_hazard_off, guard);

  return STATUS_CODE_OK;
}
