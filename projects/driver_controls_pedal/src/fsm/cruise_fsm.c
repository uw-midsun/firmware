#include "cruise_fsm.h"

#include "cruise.h"
#include "drive_output.h"
#include "pedal_events.h"

#include "log.h"

// Retains target speed
FSM_DECLARE_STATE(cruise_off);
FSM_DECLARE_STATE(cruise_on);
// Throttle is in brake zone - assume the throttle is released
// Used to allow pressing the throttle to cancel cruise
FSM_DECLARE_STATE(cruise_ready);

// 1. Switch the Digital Input to CC_ON
// Since Hitting Set will commit the current speed to the target speed.
//
// Actually marked as "on"
FSM_STATE_TRANSITION(cruise_off) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED, cruise_off);

  // TODO(ELEC-431): guard transition if target speed is 0 or below minimum?
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_ON, cruise_ready);
}

FSM_STATE_TRANSITION(cruise_ready) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED, cruise_ready);

  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
  // Exit cruise if the mechanical brake is pressed or the throttle is pressed
  // to the accel zone after entering the brake zone.
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_MECH_BRAKE_PRESSED, cruise_ready);

  // Revert back to cruise off on power off/fault
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_POWER_STATE_OFF, cruise_off);
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_POWER_STATE_FAULT, cruise_off);
}

// Cruise resumed
FSM_STATE_TRANSITION(cruise_on) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED, cruise_on);

  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
  // Exit cruise if the mechanical brake is pressed.
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_MECH_BRAKE_PRESSED, cruise_ready);

  // Revert back to cruise off on power off/fault
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_POWER_STATE_OFF, cruise_off);
  FSM_ADD_TRANSITION(PEDAL_EVENT_INPUT_POWER_STATE_FAULT, cruise_off);
}

static void prv_cruise_off_output(Fsm *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();

  // Off - Keep target speed, but send disabled to motor controllers
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, 0);
}

static void prv_cruise_on_output(Fsm *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  // Cruise control enabled - send target
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, cruise_get_target_cms(cruise));
  LOG_DEBUG("Cruise: On\n");
}

static void prv_cruise_ready_output(Fsm *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  // Cruise control enabled - send target
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, cruise_get_target_cms(cruise));
  LOG_DEBUG("Cruise: Ready\n");
}

StatusCode cruise_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(cruise_off, prv_cruise_off_output);
  fsm_state_init(cruise_on, prv_cruise_on_output);
  fsm_state_init(cruise_ready, prv_cruise_ready_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, NULL);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Cruise FSM", &cruise_off, guard);

  return STATUS_CODE_OK;
}
