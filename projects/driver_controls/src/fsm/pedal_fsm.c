// Updates to the drive output module are driven by the update requested events
// The throttle module should continuously provide updates to move between states, but its data
// will be polled on state outputs. This is so that we can handle mechanical braking and cruise
// control properly. Since the throttle module is still providing updates, state changes will cause
// drive output updates to ensure that major changes are reflected in the periodic updates.
// If the throttle is not in a braking state while the mechanical brakes are pressed, it will be
// set to coast.
// TODO: is this the right way of doing it? seems to pull in dependencies that we don't need to
// since the throttle module should already be raising events
// however the mechanical brake only raises one event
// if we're in the brake state due to mechanical braking or in the cruise control state, we rely on
// the update request to update the drive output module.
// We probably still want to support regen braking if mechanical braking is active
// todo: move cruise control out of the pedal FSM?
// We can exit cruise control by entering coast mode on the throttle
#include "pedal_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "input_event.h"
#include "log.h"

// Pedal FSM state definitions

FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_driving);
FSM_DECLARE_STATE(state_cruise_control);
// Entered braking state while in cruise
FSM_DECLARE_STATE(state_cruise_braking);

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_brake);

  // Press the gas pedal to start moving the car
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_coast);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_PRESSED, state_driving);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_driving);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_cruise_control);
}

FSM_STATE_TRANSITION(state_cruise_control) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_cruise_control);

  // Since the cruise control increase/decrease events have information that needs to be output to
  // CAN, they will cause repeat transitions
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_INC, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_DEC, state_cruise_control);

  // Cruise control exits either through hitting the cruise control switch or by engaging the
  // mechanical brake
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  // We enter coast as a safe default, but the next throttle command should update the proper state.
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_coast);
  // If we enter the brake state, we'll still be in cruise but we want to allow exit through coast
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_cruise_braking);
}

FSM_DECLARE_STATE(state_cruise_braking) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_cruise_control);

  // Since the cruise control increase/decrease events have information that needs to be output to
  // CAN, they will cause repeat transitions
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_INC, state_cruise_control);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_DEC, state_cruise_control);

  // Cruise control exits either through hitting the cruise control switch or by engaging the
  // mechanical brake
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  // We enter coast as a safe default, but the next throttle command should update the proper state.
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_coast);
  // We allow exiting cruise control by entering coast mode after we've released the pedal
  // We do not allow exiting directly into brake to prevent extremely strong braking on cruise disable
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  // Note that the direction FSM should prevent the power from being turned off if the car is
  // capable of moving
  DriveOutputStorage *storage = drive_output_global();

  // TODO: need to figure out where brake lights are configured

  // TODO: steering angle sensor isn't updated by anything yet
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_STEERING_ANGLE, 321);

  if (fsm->current_state == &state_cruise_control) {
    // TODO: actually get target value
    drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, 1234);
    // Ignore throttle data - default to neutral just in case
    drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, 0);
  } else {
    // Make sure cruise is disabled
    drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, DRIVE_OUTPUT_INVALID_CRUISE_VELOCITY);
    // TODO: Actually get throttle percentage
    drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE,
                        (fsm->current_state == &state_brake) ? -1234 : 1234);
  }
}

static void prv_cruise_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();

  // TODO actually get target value
  // set throttle position to 0? might be nice to keep the actual position for debug
  // TODO: start with current speed
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, 1234);
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, 0);
}

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_cruise_output);
  fsm_state_init(state_cruise_braking, prv_cruise_output);

  EventArbiter *arbiter = event_arbiter_add_fsm(storage, fsm, NULL);

  if (arbiter == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "pedal_fsm", &state_brake, arbiter);

  return STATUS_CODE_OK;
}
