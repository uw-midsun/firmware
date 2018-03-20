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

static bool prv_guard_engaged(const Event *e) {
  // While the brakes are engaged, the car should not accept any commands to exit braking state
  switch (e->id) {
    case INPUT_EVENT_PEDAL_COAST:
    case INPUT_EVENT_PEDAL_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
      return false;
    default:
      return true;
  }
}

static bool prv_guard_disengaged(const Event *e) {
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

static void prv_engaged_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_guard_engaged);
}

static void prv_disengaged_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_guard_disengaged);
}

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_engaged, prv_engaged_output);
  fsm_state_init(state_disengaged, prv_disengaged_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_disengaged);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "mechanical_brake_fsm", &state_disengaged, guard);

  return STATUS_CODE_OK;
}
