#include "cruise_fsm.h"
#include "cruise.h"
#include "steering_output.h"
#include "sc_input_event.h"

// Retains target speed
FSM_DECLARE_STATE(cruise_off);
FSM_DECLARE_STATE(cruise_on);
// Throttle is in brake zone - assume the throttle is released
// Used to allow pressing the throttle to cancel cruise
FSM_DECLARE_STATE(cruise_on_brake);

// Actually marked as "on"
FSM_STATE_TRANSITION(cruise_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_UPDATE_REQUESTED, cruise_off);

  // TODO(ELEC-431): guard transition if target speed is 0 or below minimum?
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, cruise_on);
}

// Cruise resumed
FSM_STATE_TRANSITION(cruise_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_UPDATE_REQUESTED, cruise_on);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_off);
  // Exit cruise if the mechanical brake is pressed.
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_MECHANICAL_BRAKE_PRESSED, cruise_off);
  // If the throttle enters the brake zone, consider the throttle as released
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_PEDAL_BRAKE, cruise_on_brake);

  // Revert back to cruise off on power off/fault
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_POWER_STATE_OFF, cruise_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_POWER_STATE_FAULT, cruise_off);
}

FSM_STATE_TRANSITION(cruise_on_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_UPDATE_REQUESTED, cruise_on_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_off);
  // Exit cruise if the mechanical brake is pressed or the throttle is pressed to
  // the accel zone after entering the brake zone.
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_MECHANICAL_BRAKE_PRESSED, cruise_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_PEDAL_ACCEL, cruise_off);

  // Revert back to cruise off on power off/fault
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_POWER_STATE_OFF, cruise_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_STEERING_POWER_STATE_FAULT, cruise_off);
}

static void prv_cruise_off_output(Fsm *fsm, const Event *e, void *context) {
  SteeringOutputStorage *storage = steering_output_global();

  // Off - Keep target speed, but send disabled to motor controllers
  steering_output_update(storage, STEERING_OUTPUT_SOURCE_CRUISE, 0);
}

static void prv_cruise_on_output(Fsm *fsm, const Event *e, void *context) {
  SteeringOutputStorage *storage = steering_output_global();
  CruiseStorage *cruise = cruise_global();

  // Cruise control enabled - send target
  steering_output_update(storage, STEERING_OUTPUT_SOURCE_CRUISE, cruise_get_target_cms(cruise));
}

StatusCode cruise_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(cruise_off, prv_cruise_off_output);
  fsm_state_init(cruise_on, prv_cruise_on_output);
  fsm_state_init(cruise_on_brake, prv_cruise_on_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Cruise FSM", &cruise_off, guard);

  return STATUS_CODE_OK;
}
