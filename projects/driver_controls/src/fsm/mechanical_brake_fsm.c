#include "mechanical_brake_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Mechanical Brake FSM state definitions

FSM_DECLARE_STATE(state_engaged);
FSM_DECLARE_STATE(state_disengaged);

// Mechanical Brake FSM transition table definitions

FSM_STATE_TRANSITION(state_engaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

FSM_STATE_TRANSITION(state_disengaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

// Mechanical Brake FSM arbiter functions

static bool prv_check_mechanical_brake_engaged(const Event *e) {
  // While the brakes are engaged, the car should not accept any commands to exit braking state
  switch (e->id) {
    case INPUT_EVENT_PEDAL_COAST:
    case INPUT_EVENT_PEDAL_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
      // case INPUT_EVENT_CRUISE_CONTROL_INC:
      // case INPUT_EVENT_CRUISE_CONTROL_DEC:
      return false;
    default:
      return true;
  }
}

static bool prv_check_mechanical_brake_disengaged(const Event *e) {
  // The brake must be engaged in order for gear shifts to happen.
  switch (e->id) {
    case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:
    case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:
    case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
      return false;
    default:
      return true;
  }
}

// Mechanical Brake FSM output functions

static void prv_state_mechanical_brake_engaged(FSM *fsm, const Event *e, void *context) {
  EventArbiter *arbiter = fsm->context;
  event_arbiter_set_event_check(arbiter, prv_check_mechanical_brake_engaged);

  // // Output brake state and angle data
  // EventArbiterOutputData data = { .id = CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE,
  //                                 .state = MECHANICAL_BRAKE_FSM_STATE_ENGAGED,
  //                                 .data = e->data };

  // event_arbiter_output(data);
}

static void prv_state_mechanical_brake_disengaged(FSM *fsm, const Event *e, void *context) {
  EventArbiter *arbiter = fsm->context;
  event_arbiter_set_event_check(arbiter, prv_check_mechanical_brake_disengaged);

  // // Output brake state and angle data
  // EventArbiterOutputData data = { .id = CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE,
  //                                 .state = MECHANICAL_BRAKE_FSM_STATE_DISENGAGED,
  //                                 .data = e->data };

  // event_arbiter_output(data);
}

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_engaged, prv_state_mechanical_brake_engaged);
  fsm_state_init(state_disengaged, prv_state_mechanical_brake_disengaged);

  EventArbiter *arbiter =
      event_arbiter_add_fsm(storage, fsm, prv_check_mechanical_brake_disengaged);

  if (arbiter == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "mechanical_brake_fsm", &state_disengaged, arbiter);

  return STATUS_CODE_OK;
}
