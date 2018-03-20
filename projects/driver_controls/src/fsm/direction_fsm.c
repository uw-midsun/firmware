// Responds to drive output update requests by updating the associated data
// Note that this is accomplished by transitioning back to the current state.
#include "direction_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Direction selector FSM state definitions

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

// Direction selector FSM transition table definitions

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_neutral);

  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_forward);

  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_reverse);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_reverse);

  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_neutral);
}

// Direction selector FSM arbiter guard functions

static bool prv_guard_neutral(const Event *e) {
  // The car should always be coasting when in the neutral state
  // TODO: make sure the "update" event doesn't bypass this check? or make sure motor controller
  // interface checks
  switch (e->id) {
    case INPUT_EVENT_PEDAL_BRAKE:
    case INPUT_EVENT_PEDAL_PRESSED:
    case INPUT_EVENT_CRUISE_CONTROL:
      return false;
    default:
      return true;
  }
}

static bool prv_guard_forward(const Event *e) {
  // Powering off while the car is moving can be dangerous. As a result, powering off is
  // forbidden while in forward gear
  return (e->id != INPUT_EVENT_POWER);
}

static bool prv_guard_reverse(const Event *e) {
  // Same as forward, except that cruise control is forbidden in reverse for obvious reasons
  return !(e->id == INPUT_EVENT_POWER || e->id == INPUT_EVENT_CRUISE_CONTROL);
}

// Direction selector FSM output functions

static void prv_neutral_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_DIRECTION,
                      DRIVE_OUTPUT_DIRECTION_NEUTRAL);
  event_arbiter_set_guard_fn(guard, prv_guard_neutral);
}

static void prv_forward_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_DIRECTION,
                      DRIVE_OUTPUT_DIRECTION_FORWARD);
  event_arbiter_set_guard_fn(guard, prv_guard_forward);
}

static void prv_reverse_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_DIRECTION,
                      DRIVE_OUTPUT_DIRECTION_REVERSE);
  event_arbiter_set_guard_fn(guard, prv_guard_reverse);
}

StatusCode direction_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_neutral, prv_neutral_output);
  fsm_state_init(state_forward, prv_forward_output);
  fsm_state_init(state_reverse, prv_reverse_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_neutral);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // TODO: should we start in neutral or forward? might need to load new direction at init
  fsm_init(fsm, "direction_fsm", &state_neutral, guard);

  return STATUS_CODE_OK;
}
