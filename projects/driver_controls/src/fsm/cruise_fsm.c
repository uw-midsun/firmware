#include "cruise_fsm.h"
#include "input_event.h"

// Clears target speed
FSM_DECLARE_STATE(cruise_off);
// Retains target speed
FSM_DECLARE_STATE(cruise_idle);
FSM_DECLARE_STATE(cruise_on);
// Throttle is in brake zone - assume the throttle is released
// Used to allow pressing the throttle to cancel cruise
FSM_DECLARE_STATE(cruise_on_brake);

FSM_STATE_TRANSITION(cruise_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON, cruise_idle);
}

// Actually marked as "on"
FSM_STATE_TRANSITION(cruise_idle) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_idle);

  // TODO: guard transition if target speed is 0 or below minimum?
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, cruise_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
}

// Cruise resumed
FSM_STATE_TRANSITION(cruise_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_on);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_idle);
  // Exit cruise if the mechanical brake is pressed.
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, cruise_idle);
  // If the throttle enters the brake zone, consider the throttle as released
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, cruise_on_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
}

FSM_DECLARE_STATE(cruise_on_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_on);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_idle);
  // Exit cruise if the mechanical brake is pressed or the throttle is pressed to
  // the accel zone after entering the brake zone.
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, cruise_idle);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, cruise_idle);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
}

static void prv_cruise_off_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  // Clear target speed
  cruise_set_target_cms(cruise, 0);
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, 0);
}

static void prv_cruise_idle_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();

  // Off - Keep target speed, but send disabled to motor controllers
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, 0);
}

static void prv_cruise_on_output(FSM *fsm, const Event *e, void *context) {
  DriveOutputStorage *storage = drive_output_global();
  CruiseStorage *cruise = cruise_global();

  // Cruise control enabled - send target
  drive_output_update(storage, DRIVE_OUTPUT_SOURCE_CRUISE, cruise_get_target_cms(cruise));
}
