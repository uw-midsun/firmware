// The driver uses the mechanical brake to control the powered state of the car.

// The car initializes in the off state:
//    - Pressing the power button without holding down the brake will cause the
//    car
//      to transition from the off to the charging state, and vice versa
//    - Pressing the power button while the mechanical brake is held down will
//    cause the car
//      to transition between the off and the on state.

#include "power_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// Power FSM state definitions

FSM_DECLARE_STATE(state_off);
FSM_DECLARE_STATE(state_off_brake);
FSM_DECLARE_STATE(state_charging);
FSM_DECLARE_STATE(state_charging_brake);
FSM_DECLARE_STATE(state_on);

// Power FSM transition table definitions

FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_charging);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_off_brake);
}

FSM_STATE_TRANSITION(state_off_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_off);
}

FSM_STATE_TRANSITION(state_charging) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_charging_brake);
}

FSM_STATE_TRANSITION(state_charging_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, state_charging);
}

FSM_STATE_TRANSITION(state_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER, state_off);
}

// Power FSM guard functions
static bool prv_check_off(const Event *e) {
  // The car must accept a command to power on while off. It must also
  // acknowledge mechanical brake
  // events, as the mechanical brake does not rely on the car being powered.
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

static void prv_state_off(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, prv_check_off);

  PowerFSMState power_state;
  State *current_state = fsm->current_state;

  // If a transition has happened between the FSMs main states, a CAN message
  // will be sent out.
  if (current_state == &state_off) {
    power_state = POWER_FSM_STATE_OFF;
  } else if (current_state == &state_charging) {
    power_state = POWER_FSM_STATE_CHARGING;
  } else if (current_state == &state_on) {
    power_state = POWER_FSM_STATE_ON;
  } else {
    // state_off_brake and state_charging_brake are simply substates of
    // state_off and
    // state_charging respectively. No CAN message gets sent out
    return;
  }

  (void)power_state;
  // Previous: Output Power off
}

static void prv_state_on(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = fsm->context;
  event_arbiter_set_guard_fn(guard, NULL);

  // Previous: Output Power on
}

StatusCode power_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_off, prv_state_off);
  fsm_state_init(state_off_brake, prv_state_off);
  fsm_state_init(state_charging, prv_state_off);
  fsm_state_init(state_charging_brake, prv_state_off);
  fsm_state_init(state_on, prv_state_on);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_check_off);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "power_fsm", &state_off, guard);

  return STATUS_CODE_OK;
}
