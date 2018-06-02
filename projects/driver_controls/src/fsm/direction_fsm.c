// Responds to drive output update requests by updating the associated data
// Note that this is accomplished by transitioning back to the current state.
#include "direction_fsm.h"
#include <stddef.h>
#include "drive_output.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "input_event.h"
#include "log.h"

// Direction selector FSM state definitions

FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_reverse);

// Direction selector FSM transition table definitions

FSM_STATE_TRANSITION(state_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_forward);

  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_reverse);
}

// TODO(ELEC-407): Add neutral state

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_reverse);

  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_forward);
  // TODO(ELEC-407): revert to default state on power on?
}

// Direction selector FSM arbiter guard functions

static bool prv_guard_reverse(const Event *e) {
  // Cruise control is forbidden in reverse for obvious reasons
  return e->id != INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME;
}

// Direction selector FSM output functions

static void prv_forward_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_DIRECTION,
                      EE_DRIVE_OUTPUT_DIRECTION_FORWARD);
  event_arbiter_set_guard_fn(guard, NULL);
}

static void prv_reverse_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_DIRECTION,
                      EE_DRIVE_OUTPUT_DIRECTION_REVERSE);
  event_arbiter_set_guard_fn(guard, prv_guard_reverse);
}

StatusCode direction_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_forward, prv_forward_output);
  fsm_state_init(state_reverse, prv_reverse_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Direction FSM", &state_forward, guard);

  return STATUS_CODE_OK;
}
