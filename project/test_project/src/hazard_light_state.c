#include "hazard_light_state.h"

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// Transition table for the turn signal state machine

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT_OFF, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_HAZARD_LIGHT_ON, state_hazard_on);
}

// Output functions for the hazard light state

static void prv_driver_state_hazard_on(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_HAZARD_ON;
}

static void prv_driver_state_hazard_off(FSM* fsm, const Event* e, void *context) {
  *(uint8_t*)context = STATE_HAZARD_OFF;
}

void hazard_light_state_init(FSM* hazard_light_fsm, FSMState* state_id) {
	fsm_state_init(state_hazard_on, prv_driver_state_hazard_on);
	fsm_state_init(state_hazard_off, prv_driver_state_hazard_off);

  fsm_init(hazard_light_fsm, "hazard_light_fsm", &state_hazard_off, state_id);
}