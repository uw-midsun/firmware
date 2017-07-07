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

// Transition check functions
static bool prv_check_mechanical_brake_engaged(const Event *e) {
  switch (e->id) {
    case INPUT_EVENT_GAS_COAST:
    case INPUT_EVENT_GAS_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
    case INPUT_EVENT_CRUISE_CONTROL_INC:
    case INPUT_EVENT_CRUISE_CONTROL_DEC:
      return false;
    default:
      return true;
  }
}

static bool prv_check_mechanical_brake_disengaged(const Event *e) {
  switch (e->id) {
    case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:
    case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:
    case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
      return false;
    default:
      return true;
  }
}

// State output functions
static void prv_state_mechanical_brake_engaged(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_mechanical_brake_engaged;
}

static void prv_state_mechanical_brake_disengaged(FSM *fsm, const Event *e, void *context) {
  fsm->context = prv_check_mechanical_brake_disengaged;
}

void mechanical_brake_state_init(FSM *power_fsm, void *context) {
  fsm_state_init(state_engaged, prv_state_mechanical_brake_engaged);
  fsm_state_init(state_disengaged, prv_state_mechanical_brake_disengaged);

  fsm_init(power_fsm, "power_fsm", &state_disengaged, prv_check_mechanical_brake_disengaged);
}
