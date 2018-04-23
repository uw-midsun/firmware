#include "pedal_calibration_fsm.h"
#include "input_event.h"
#include "log.h"

FSM_DECLARE_STATE(state_start);
FSM_DECLARE_STATE(state_full_brake);
FSM_DECLARE_STATE(state_full_throttle);
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
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
}

FSM_STATE_TRANSITION(state_validate) {
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_VALIDATE, state_validate);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_CALIBRATION_START, state_start);
}

static void prv_start_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("You are at the start of pedal calibration.\n");
}

static void prv_full_brake_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("Keep the pedal at full brake position.\n");
  PedalCalibrationStorage *storage = context;
  pedal_calibration_process_state(storage, PEDAL_CALIBRATION_STATE_FULL_BRAKE);
  LOG_DEBUG("Full brake stage complete.\n");
}

static void prv_full_throttle_output(FSM *fsm, const Event *e, void *context) {
  LOG_DEBUG("Keep the pedal at full throttle position.\n");
  PedalCalibrationStorage *storage = context;
  pedal_calibration_process_state(storage, PEDAL_CALIBRATION_STATE_FULL_THROTTLE);
  LOG_DEBUG("Full throttle stage complete.\n");
}

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

StatusCode pedal_calibration_fsm_init(FSM *fsm, PedalCalibrationStorage *calibration_storage) {
  fsm_state_init(state_start, prv_start_output);
  fsm_state_init(state_full_brake, prv_full_brake_output);
  fsm_state_init(state_full_throttle, prv_full_throttle_output);
  fsm_state_init(state_validate, prv_validate_output);

  fsm_init(fsm, "pedal_calibration_fsm", &state_start, &calibration_storage);

  return STATUS_CODE_OK;
}
