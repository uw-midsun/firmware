// The driver uses the mechanical brake to control the powered state of the car.

// The car initializes in the off state:
//    - Pressing the power button without holding down the brake will cause the car
//      to transition from the off to the charging state, and vice versa
//    - Pressing the power button while the mechanical brake is held down will cause the car
//      to transition between the off and the on state.

#include "power_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Power FSM state definitions

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_off_brake);
FSM_DECLARE_STATE(state_charging);
FSM_DECLARE_STATE(state_charging_brake);
FSM_DECLARE_STATE(state_on);
FSM_DECLARE_STATE(state_fault);

// Power FSM transition table definitions

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_charging);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_off_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_off_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_charging) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_charging_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_charging_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_charging);

  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_BPS_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
}

// Power FSM arbiter functions
static bool prv_guard_off(const Event *e) {
  // The only valid events when the car isn't in drive are the power button and mechanical brake.
  // This also prevents lights, etc. from being turned on unless the unprotected rail is powered.
  switch (e->id) {
    case INPUT_EVENT_POWER:
    case INPUT_EVENT_MECHANICAL_BRAKE_RELEASED:
    case INPUT_EVENT_MECHANICAL_BRAKE_PRESSED:
      return true;
    default:
      return false;
  }
}

// Power FSM output functions

static void prv_on_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;

  // Allow all events and begin sending periodic drive commands
  drive_output_set_enabled(drive_output_global(), true);
  event_arbiter_set_guard_fn(guard, NULL);
}

static void prv_off_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;

  // Disable periodic drive output updates if not running
  drive_output_set_enabled(drive_output_global(), false);
  event_arbiter_set_guard_fn(guard, prv_guard_off);
}

StatusCode power_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_off, prv_off_output);
  fsm_state_init(state_off_brake, NULL);
  fsm_state_init(state_charging, prv_off_output);
  fsm_state_init(state_charging_brake, NULL);
  fsm_state_init(state_on, prv_on_output);
  fsm_state_init(state_fault, prv_off_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_guard_off);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "power_fsm", &state_off, guard);

  return STATUS_CODE_OK;
}
