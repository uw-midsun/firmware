// Updates to the drive output module are driven by the update requested events
// The throttle module should continuously provide updates to move between states, but its data
// will be polled on state outputs. This is so that we can handle mechanical braking and cruise
// control properly. Since the throttle module is still providing updates, state changes will cause
// drive output updates to ensure that major changes are reflected in the periodic updates.

// The overall operation of the car is governed by the FSMs. They have an implicit hierarchy:
// * Power FSM only allows power/mechanical braking when off
// * Mechanical brake FSM prevents exiting braking state when engaged and only allows
//   gear shifts while engaged
// * Direction FSM prevents turning off the car when in drive or reverse and prevents
//   drive/braking in neutral
// * Pedal FSM is at the bottom - after all operations are filtered, drive output updates
//   occur in the FSM

#include "pedal_fsm.h"
#include "cruise.h"
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

FSM_STATE_TRANSITION(state_cruise_braking) {
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
  // We do not allow exiting directly into brake to prevent extremely strong braking on cruise
  // disable
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  cruise_set_source(cruise, CRUISE_SOURCE_MOTOR_CONTROLLER);

  // TODO: handle brake signal lights somewhere
  // Make sure cruise is disabled
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, DRIVE_OUTPUT_CRUISE_DISABLED_SPEED);
  // TODO: Actually get throttle percentage - this will not depend on the current state
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE,
                      (fsm->current_state == &state_brake) ? -1234 : 1234);
}

static void prv_cruise_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  cruise_set_source(cruise, CRUISE_SOURCE_STORED_VALUE);

  // Cruise is enabled - since we're using the stored source, the target can now be modified
  // by the INC/DEC inputs
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, cruise_get_target(cruise));
  // TODO: get throttle state
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, 0);
}

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_cruise_output);
  fsm_state_init(state_cruise_braking, prv_cruise_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "pedal_fsm", &state_brake, guard);

  return STATUS_CODE_OK;
}
