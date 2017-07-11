#include "power_state.h"

FSM_DECLARE_STATE(state_engaged);
FSM_DECLARE_STATE(state_disengaged);

// State machine transition tables
FSM_STATE_TRANSITION(state_engaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE, state_disengaged);
}

FSM_STATE_TRANSITION(state_disengaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE, state_engaged);
}

// State output functions
static void prv_state_mechanical_brake_engaged(FSM *fsm, const Event *e, void *context) {
  switch (e->id) {
    case INPUT_EVENT_GAS_COAST:
    case INPUT_EVENT_GAS_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
    case INPUT_EVENT_CRUISE_CONTROL_INC:
    case INPUT_EVENT_CRUISE_CONTROL_DEC:
      *(bool*)fsm->context = false;
    default:
      *(bool*)fsm->context = true;
  }
}

static void prv_state_mechanical_brake_disengaged(FSM *fsm, const Event *e, void *context) {
  
  switch (e->id) {
    case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:
    case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:
    case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
      *(bool*)fsm->context = false;
    default:
      *(bool*)fsm->context = true;
  }
}

void mechanical_brake_state_init(FSM *power_fsm, void *context) {
  fsm_state_init(state_engaged, prv_state_mechanical_brake_engaged);
  fsm_state_init(state_disengaged, prv_state_mechanical_brake_disengaged);

  bool approval;
  fsm_init(power_fsm, "power_fsm", &state_disengaged, &approval);
}
