// Updates to the drive output module are driven by the update requested events

#include "mechanical_brake_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Mechanical Brake FSM state definitions

FSM_DECLARE_STATE(state_engaged);
FSM_DECLARE_STATE(state_disengaged);

// Mechanical Brake FSM transition table definitions

FSM_STATE_TRANSITION(state_engaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_engaged);

  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

FSM_STATE_TRANSITION(state_disengaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_disengaged);

  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

// Mechanical Brake FSM arbiter functions

static bool prv_guard_engaged(const Event *e) {
  // While the brakes are engaged, the car shouldn't allow the car to enter cruise control.
  // Motor controller interface should ignore throttle state if mechanical brake is engaged.
  switch (e->id) {
    case INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME:
      return false;
    default:
      return true;
  }
}

static bool prv_guard_disengaged(const Event *e) {
  // The brake must be engaged in order for gear shifts to happen.
  switch (e->id) {
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

  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_MECH_BRAKE, 0);
}

static void prv_disengaged_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_guard_disengaged);

  // TODO(ELEC-350): Implement mech brake
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_MECH_BRAKE, 0);
}

StatusCode mechanical_brake_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_engaged, prv_engaged_output);
  fsm_state_init(state_disengaged, prv_disengaged_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_disengaged);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Mechanical Brake FSM", &state_disengaged, guard);

  return STATUS_CODE_OK;
}
