#include "pedal_calibration_fsm.h"
#include "input_event.h"
#include "log.h"

FSM_DECLARE_STATE(state_start);
FSM_DECLARE_STATE(state_full_brake);
FSM_DECLARE_STATE(state_full_throttle);
FSM_DECLARE_STATE(state_validate);

FSM_STATE_TRANSITION(state_start) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_BRAKE, state_full_brake);

  // User can go to validation stage if already calibrated.
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
}

FSM_STATE_TRANSITION(state_full_brake) {
  // The only available next step is full throttle state.
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_THROTTLE, state_full_throttle);

  // Or repeating full brake stage if pedal was not in the correct position at some point.
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_BRAKE, state_full_brake);
}

FSM_STATE_TRANSITION(state_full_throttle) {
  // The only available next step is validate state. The user must validate the calibration.
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);

  // Or repeating full throttle stage if pedal was not in the correct position at some point.
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_FULL_THROTTLE, state_full_throttle);
}

FSM_STATE_TRANSITION(state_validate) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_START, state_start);
}

// Output function for the start state.
static void prv_start_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("You are at the menu of pedal calibration.\n");
}

// Output function for the full brake state.
// Reads and collects pedal inputs assuming pedal is in full brake position.
static void prv_full_brake_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("Keep the pedal at full brake position.\n");
  PedalCalibrationStorage *storage = context;
  pedal_calibration_process_state(storage, PEDAL_CALIBRATION_STATE_FULL_BRAKE);
  LOG_DEBUG("Full brake stage complete.\n");
}

// Output function for the full throttle state.
// Reads and collects pedal inputs assuming pedal is in full throttle position.
static void prv_full_throttle_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("Keep the pedal at full throttle position.\n");
  PedalCalibrationStorage *storage = context;
  pedal_calibration_process_state(storage, PEDAL_CALIBRATION_STATE_FULL_THROTTLE);
  LOG_DEBUG("Full throttle stage complete.\n");
}

// Output function for the validate state.
// Note that the calculations on the collected data takes place here.
// Validation process continues until fsm has a new transition.
// It constantly logs the current zone info of the pedal.
static void prv_validate_output(FSM *fsm, const Event *e, void *context) {
  PedalCalibrationStorage *storage = context;
  pedal_calibration_calculate(storage, storage->throttle_calibration_data);

  LOG_DEBUG("Use the pedal to validate the calibration.\n");

  ThrottleStorage throttle;
  ThrottlePosition position = { .zone = THROTTLE_ZONE_BRAKE, .numerator = 0 };
  throttle_init(&throttle, storage->throttle_calibration_data, storage->ads1015_storage);

  Event new_event;
  do {
    event_process(&new_event);
    throttle_get_position(&throttle, &position);
    char *zones[NUM_THROTTLE_ZONES] = { [THROTTLE_ZONE_BRAKE] = "Brake",
                                        [THROTTLE_ZONE_COAST] = "Coast",
                                        [THROTTLE_ZONE_ACCEL] = "Accel" };

    LOG_DEBUG("%s zone: %d / %d OR %d percent\n", zones[position.zone], position.numerator,
              THROTTLE_DENOMINATOR, position.numerator * 100 / THROTTLE_DENOMINATOR);
  } while (!fsm_process_event(fsm, &new_event));
}

// Initializes the fsm with the corresponding output functions.
StatusCode pedal_calibration_fsm_init(FSM *fsm, PedalCalibrationStorage *calibration_storage) {
  fsm_state_init(state_start, prv_start_output);
  fsm_state_init(state_full_brake, prv_full_brake_output);
  fsm_state_init(state_full_throttle, prv_full_throttle_output);
  fsm_state_init(state_validate, prv_validate_output);

  fsm_init(fsm, "pedal_calibration_fsm", &state_start, &calibration_storage);

  return STATUS_CODE_OK;
}
