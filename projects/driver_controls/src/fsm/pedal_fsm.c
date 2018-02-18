// Responds to drive output update requests by updating the associated data
// Note that this is accomplished by transitioning back to the current state.
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
  // TODO: should we allow cruise control to be started from neutral?
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
  // TODO: these should be moved somewhere else?
  // FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_INC, state_cruise_control);
  // FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL_DEC, state_cruise_control);

  // Cruise control exits either through hitting the cruise control switch or by engaging the
  // mechanical brake
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_CRUISE_CONTROL, state_brake);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();

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
    drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, 1234);
  }
}

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);
  fsm_state_init(state_cruise_control, prv_state_output);

  EventArbiter *arbiter = event_arbiter_add_fsm(storage, fsm, NULL);

  if (arbiter == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "pedal_fsm", &state_brake, arbiter);

  return STATUS_CODE_OK;
}
