#include "cruise_fsm.h"
#include "input_event.h"

// Clears target speed
FSM_DECLARE_STATE(cruise_off);
// Retains target speed
FSM_DECLARE_STATE(cruise_idle);
FSM_DECLARE_STATE(cruise_on);

FSM_STATE_TRANSITION(cruise_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_off);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON, cruise_idle);
}

// Actually marked as "on"
FSM_STATE_TRANSITION(cruise_idle) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_idle);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME, cruise_on);
  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF, cruise_off);
}

// Cruise resumed
FSM_STATE_TRANSITION(cruise_on) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, cruise_on);

  FSM_ADD_TRANSITION(INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL, cruise_idle);
  FSM_ADD_TRANSITION(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, cruise_idle);
  // TODO: handle throttle press to deactivate cruise
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
