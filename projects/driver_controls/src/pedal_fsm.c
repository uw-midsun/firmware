#include "pedal_fsm.h"
#include "input_event.h"
#include "event_arbiter.h"
#include "log.h"

// Pedal FSM state definitions

FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);
FSM_DECLARE_STATE(state_cruise_control);

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_INC, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_DEC, state_cruise_control);
}

// Pedal FSM arbiter function

static bool prv_check_pedal(const Event *e) {
  return true;
}

// Pedal FSM output functions

static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterCheck *event_check = fsm->context;
  *event_check = prv_check_pedal;

  switch (e->id) {
    case INPUT_EVENT_PEDAL_BRAKE:
    case INPUT_EVENT_PEDAL_COAST:
    case INPUT_EVENT_PEDAL_PRESSED:
      LOG_DEBUG("Pedal in %s\n", fsm->current_state->name);
    case INPUT_EVENT_CRUISE_CONTROL:
      LOG_DEBUG("Car in %s\n", fsm->current_state->name);
    case INPUT_EVENT_CRUISE_CONTROL_INC:
      LOG_DEBUG("Cruise control increase speed\n");
    case INPUT_EVENT_CRUISE_CONTROL_DEC:
      LOG_DEBUG("Cruise control decrease speed\n");
  }
}

StatusCode pedal_fsm_init(FSM *fsm) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_state_output);

  fsm_init(fsm, "pedal_fsm", &state_brake, NULL);

  fsm->context = event_arbiter_add_fsm(fsm, prv_check_pedal);

  if (fsm->context == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  return STATUS_CODE_OK;
}
