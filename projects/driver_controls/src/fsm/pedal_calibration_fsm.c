#include "pedal_calibration_fsm.h"
#include "input_event.h"

FSM_DECLARE_STATE(state_start);
// FSM_DECLARE_STATE(state_end);
FSM_DECLARE_STATE(state_full_brake);
FSM_DECLARE_STATE(state_full_throttle);
FSM_DECLARE_STATE(state_calculate);
FSM_DECLARE_STATE(state_validate);

FSM_STATE_TRANSITION(state_start) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_BRAKE, state_full_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
}

FSM_STATE_TRANSITION(state_full_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_BRAKE, state_full_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_THROTTLE, state_full_throttle);
}

FSM_STATE_TRANSITION(state_full_throttle) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_THROTTLE, state_full_throttle);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_CALCULATE, state_calculate);
}

FSM_STATE_TRANSITION(state_calculate) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_START, state_start);
}

FSM_STATE_TRANSITION(state_validate) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_START, state_start);
}

static void prv_start_output(FSM *fsm, const Event *e, void *context) {}
static void prv_full_brake_output(FSM *fsm, const Event *e, void *context) {}
static void prv_full_throttle_output(FSM *fsm, const Event *e, void *context) {}
static void prv_calculate_output(FSM *fsm, const Event *e, void *context) {}
static void prv_validate_output(FSM *fsm, const Event *e, void *context) {}

StatusCode pedal_calibration_fsm_init(FSM *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_start, prv_start_output);
  fsm_state_init(state_full_brake, prv_full_brake_output);
  fsm_state_init(state_full_throttle, prv_full_throttle_output);
  fsm_state_init(state_calculate, prv_calculate_output);
  fsm_state_init(state_validate, prv_validate_output);

  fsm_init(fsm, "pedal_calibration_fsm", &state_start, NULL);

  return STATUS_CODE_OK;
}
