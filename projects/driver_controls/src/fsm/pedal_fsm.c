// Updates to the drive output module are driven by the update requested events
// The throttle module should continuously provide updates to move between states, but its data
// will be polled on state outputs. This is so that we can handle mechanical braking and cruise
// control properly. Since the throttle module is still providing updates, state changes will cause
// drive output updates to ensure that major changes are reflected in the periodic updates.

// The overall operation of the car is governed by the FSMs. They have an implicit hierarchy:
// * Power FSM only allows power/mechanical braking when off
// * Mechanical brake FSM prevents exiting braking state when engaged and only allows
//   gear shifts while engaged
// * Direction FSM prevents cruise control in reverse
// * Pedal FSM is at the bottom - after all operations are filtered, drive output updates
//   occur in the FSM

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

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_brake);

  // Press the gas pedal to start moving the car
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_driving);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_coast);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_driving);
}

FSM_STATE_TRANSITION(state_driving) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_driving);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();

  // TODO(ELEC-354): handle brake signal lights somewhere
  // TODO(ELEC-350): Actually get throttle percentage - this will not depend on the current state
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE,
                      (fsm->current_state == &state_brake) ? -1234 : 1234);
  // TODO(ELEC-350): Implement mech brake
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_MECH_BRAKE, 0);
}

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_state_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_driving, prv_state_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "pedal_fsm", &state_brake, guard);

  return STATUS_CODE_OK;
}
