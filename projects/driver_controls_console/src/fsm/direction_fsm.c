// Responds to drive output update requests by updating the associated data
// Note that this is accomplished by transitioning back to the current state.
#include "direction_fsm.h"
#include <stddef.h>
#include "cc_input_event.h"
#include "console_output.h"
#include "event_arbiter.h"
#include "exported_enums.h"
#include "log.h"

// Direction selector FSM state definitions

FSM_DECLARE_STATE(state_forward);
FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_reverse);

// Direction selector FSM transition table definitions

FSM_STATE_TRANSITION(state_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONSOLE_UPDATE_REQUESTED, state_forward);

  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE, state_reverse);

  // Revert back to neutral on power off/fault
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_neutral);
}

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONSOLE_UPDATE_REQUESTED, state_neutral);

  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, state_forward);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_CONSOLE_UPDATE_REQUESTED, state_reverse);

  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE, state_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL, state_neutral);

  // Revert back to neutral on power off/fault
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_OFF, state_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_STATE_FAULT, state_neutral);
}

// Direction selector FSM arbiter guard functions
static bool prv_guard_prevent_cruise(const Event *e) {
  // Cruise control is forbidden in neutral/reverse for obvious reasons
  return e->id != INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME;
}

// Direction selector FSM output functions
static void prv_forward_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  console_output_update(console_output_global(), CONSOLE_OUTPUT_SOURCE_DIRECTION,
                        EE_CONSOLE_OUTPUT_DIRECTION_FORWARD);
  event_arbiter_set_guard_fn(guard, NULL);

  if (e->id != INPUT_EVENT_CONSOLE_UPDATE_REQUESTED) {
    event_raise(INPUT_EVENT_DIRECTION_STATE_FORWARD, 0);
    LOG_DEBUG("Forward\n");
  }
}

static void prv_neutral_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  console_output_update(console_output_global(), CONSOLE_OUTPUT_SOURCE_DIRECTION,
                        EE_CONSOLE_OUTPUT_DIRECTION_NEUTRAL);
  event_arbiter_set_guard_fn(guard, prv_guard_prevent_cruise);

  if (e->id != INPUT_EVENT_CONSOLE_UPDATE_REQUESTED) {
    event_raise(INPUT_EVENT_DIRECTION_STATE_NEUTRAL, 0);
    LOG_DEBUG("Neutral\n");
  }
}

static void prv_reverse_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  console_output_update(console_output_global(), CONSOLE_OUTPUT_SOURCE_DIRECTION,
                        EE_CONSOLE_OUTPUT_DIRECTION_REVERSE);
  event_arbiter_set_guard_fn(guard, prv_guard_prevent_cruise);

  if (e->id != INPUT_EVENT_CONSOLE_UPDATE_REQUESTED) {
    event_raise(INPUT_EVENT_DIRECTION_STATE_REVERSE, 0);
    LOG_DEBUG("Reverse\n");
  }
}

StatusCode direction_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_forward, prv_forward_output);
  fsm_state_init(state_neutral, prv_neutral_output);
  fsm_state_init(state_reverse, prv_reverse_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_prevent_cruise);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Direction FSM", &state_neutral, guard);

  return STATUS_CODE_OK;
}
