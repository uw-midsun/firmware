#include "hazard_light_state.h"

FSM_DECLARE_STATE(state_hazard_on);
FSM_DECLARE_STATE(state_hazard_off);

// Transition guard function
static bool prv_power_guard(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  bool transitioned = (fsm_group->pedal.state != STATE_OFF);
  return transitioned;
}

// State machine transition tables

FSM_STATE_TRANSITION(state_hazard_on) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, prv_power_guard, state_hazard_off);
}

FSM_STATE_TRANSITION(state_hazard_off) {
  FSM_ADD_GUARDED_TRANSITION(INPUT_EVENT_HAZARD_LIGHT, prv_power_guard, state_hazard_on);
}

// Output functions for the hazard light state

static void prv_driver_state_hazard_on(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->hazard_light.state = STATE_HAZARD_ON;
}

static void prv_driver_state_hazard_off(FSM* fsm, const Event* e, FSMGroup* fsm_group) {
  fsm_group->hazard_light.state = STATE_HAZARD_OFF;
}

void hazard_light_state_init(FSM* hazard_light_fsm, FSMGroup* fsm_group) {
	fsm_state_init(state_hazard_on, prv_driver_state_hazard_on);
	fsm_state_init(state_hazard_off, prv_driver_state_hazard_off);

  fsm_init(hazard_light_fsm, "hazard_light_fsm", &state_hazard_off, fsm_group);
}