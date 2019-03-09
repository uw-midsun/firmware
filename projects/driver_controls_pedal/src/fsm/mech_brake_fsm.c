// Updates to the pedal output module are driven by the update requested events

#include "event_arbiter.h"
#include "exported_enums.h"
#include "log.h"
#include "mech_brake.h"
#include "mech_brake_fsm.h"
#include "pc_input_event.h"
#include "pedal_output.h"

// Mechanical Brake FSM state definitions
FSM_DECLARE_STATE(state_engaged);
FSM_DECLARE_STATE(state_disengaged);

// Mechanical Brake FSM transition table definitions
FSM_STATE_TRANSITION(state_engaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_UPDATE_REQUESTED, state_engaged);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

FSM_STATE_TRANSITION(state_disengaged) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_UPDATE_REQUESTED, state_disengaged);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_PRESSED, state_engaged);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_MECHANICAL_BRAKE_RELEASED, state_disengaged);
}

// Mechanical Brake FSM arbiter functions
static bool prv_guard_engaged(const Event *e) {
  // While the brakes are engaged, the car shouldn't allow the car to enter cruise control.
  // Motor controller interface should ignore throttle state if mechanical brake is engaged.
  switch (e->id) {
    case INPUT_EVENT_PEDAL_CONTROL_STALK_ANALOG_CC_RESUME:
      return false;
    default:
      return true;
  }
}

static bool prv_guard_disengaged(const Event *e) {
  // The brake must be engaged in order for gear shifts to happen.
  // We allow shifting into neutral at any time.

  // Needs a listener - these events will not be raised locally
  switch (e->id) {
    case INPUT_EVENT_PEDAL_CENTER_CONSOLE_DIRECTION_DRIVE:
    case INPUT_EVENT_PEDAL_CENTER_CONSOLE_DIRECTION_REVERSE:
      return false;
    default:
      return true;
  }
}

// Mechanical Brake FSM output functions
static void prv_engaged_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_guard_engaged);

  int16_t position = INT16_MAX;
  if (status_ok(mech_brake_get_position(mech_brake_global(), &position))) {
    pedal_output_update(pedal_output_global(), PEDAL_OUTPUT_SOURCE_MECH_BRAKE, position);
  }
}

static void prv_disengaged_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_guard_disengaged);

  int16_t position = INT16_MAX;
  if (status_ok(mech_brake_get_position(mech_brake_global(), &position))) {
    pedal_output_update(pedal_output_global(), PEDAL_OUTPUT_SOURCE_MECH_BRAKE, position);
  }
}

StatusCode mechanical_brake_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_engaged, prv_engaged_output);
  fsm_state_init(state_disengaged, prv_disengaged_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_disengaged);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Mechanical Brake FSM", &state_disengaged, guard);

  return STATUS_CODE_OK;
}
