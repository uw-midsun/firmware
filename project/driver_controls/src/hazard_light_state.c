#include "hazard_light_state.h"
#include "input_event.h"

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// State machine transition tables

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_on);
}

static bool prv_check_hazard_light(Event *e) {
  return true;
}

// Output functions for the hazard light state
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  InputEventCheck *event_check = fsm->context;
  *event_check = prv_check_hazard_light;
}

void hazard_light_state_init(FSM *hazard_light_fsm, void *context) {
  fsm_state_init(state_hazard_on, prv_state_output);
  fsm_state_init(state_hazard_off, prv_state_output);

  fsm_init(hazard_light_fsm, "hazard_light_fsm", &state_hazard_off, context);
  prv_state_output(hazard_light_fsm, INPUT_EVENT_NONE, hazard_light_fsm->context);
}
