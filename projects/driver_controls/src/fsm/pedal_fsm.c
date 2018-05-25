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
#include "throttle.h"

// Pedal FSM state definitions

FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_accel);

// Pedal FSM transition table definitions

FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_brake);

  // Press the gas pedal to start moving the car
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_accel);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_coast);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_accel);
}

FSM_STATE_TRANSITION(state_accel) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_accel);

  // Return to brake state through either regen or mechanical brakes
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
}

#include "log.h"
static bool prv_brake_guard(const Event *e) {
  // Prevent entering cruise if braking
  LOG_DEBUG("brake guard e %d (blocking %d)\n", e->id, INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME);
  return e->id != INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME;
}

static void prv_brake_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = context;
  event_arbiter_set_guard_fn(guard, prv_brake_guard);

  DriveOutputStorage *storage = drive_output_global();

  ThrottlePosition position = { 0 };
  throttle_get_position(throttle_global(), &position);

  // Default to coast if held in brake state - probably due to mechanical brake
  // Mechanical brake can cause the throttle zone to be in a non-brake zone while in brake state
  int16_t throttle_position = 0;
  if (position.zone == THROTTLE_ZONE_BRAKE) {
    // Brake is negative
    throttle_position = -1 * position.numerator;
  }

  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, throttle_position);

  // TODO(ELEC-350): Implement mech brake
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_MECH_BRAKE, 0);
}

// Pedal FSM output functions
static void prv_state_output(FSM *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = context;
  event_arbiter_set_guard_fn(guard, NULL);

  DriveOutputStorage *storage = drive_output_global();

  ThrottlePosition position = { 0 };
  throttle_get_position(throttle_global(), &position);
  // Could just use another output state
  // Default to coast
  int16_t throttle_position = 0;
  if (position.zone == THROTTLE_ZONE_BRAKE) {
    throttle_position = position.numerator;
  }

  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_THROTTLE, throttle_position);

  // TODO(ELEC-350): Implement mech brake
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_MECH_BRAKE, 0);
}

// TODO: we don't actually distinguish between different states right now?
// Just for mechanical brake drive output - may want to just eliminate the pedal FSM
// and have a mech brake drive output

StatusCode pedal_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_brake_output);
  fsm_state_init(state_coast, prv_state_output);
  fsm_state_init(state_accel, prv_state_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Pedal FSM", &state_brake, guard);

  return STATUS_CODE_OK;
}
