#include "hazard_light_state.h"

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// State machine transition tables

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, state_hazard_on);
}

// Transition check functions
static bool prv_check_hazard(const Event *e) {
	return true;
}

// Output functions for the hazard light state
static void prv_state_hazard_on(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_hazard;
}

static void prv_state_hazard_off(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_hazard;
}

void hazard_light_state_init(FSM *hazard_light_fsm, void *context) {
  fsm_state_init(state_hazard_on, prv_state_hazard_on);
  fsm_state_init(state_hazard_off, prv_state_hazard_off);

  fsm_init(hazard_light_fsm, "hazard_light_fsm", &state_hazard_off, prv_check_hazard);
}