#include "power_state.h"
#include "input_event.h"

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_on);

// State machine transition tables
FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
}

FSM_STATE_TRANSITION(state_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
}

// State output functions
static void prv_state_off(FSM *fsm, const Event *e, void *context) {
  bool *permitted = fsm->context;
  *permitted = (e->id == INPUT_EVENT_POWER || e->id == INPUT_EVENT_MECHANICAL_BRAKE);
}

static void prv_state_on(FSM *fsm, const Event *e, void *context) {
  bool *permitted = fsm->context;
  *permitted = true;
}

void power_state_init(FSM *power_fsm, void *context) {
  fsm_state_init(state_off, prv_state_off);
  fsm_state_init(state_on, prv_state_on);

  fsm_init(power_fsm, "power_fsm", &state_off, context);
}
