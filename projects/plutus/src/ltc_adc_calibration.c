#include "ltc_adc_calibration.h"

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  // Correct for voltage offset
  storage->value.voltage = *value - storage->line->zero_point.voltage;

  // Formula for calculating calibrated current. Draws slope between given calibration
  // points, and uses the result as well as the voltage offset to calculate current
  storage->value.current = storage->line->max_point.current *
                           (*value - storage->line->zero_point.voltage) /
                           (storage->line->max_point.voltage - storage->line->zero_point.voltage);

  if (storage->callback != NULL) {
    storage->callback(&storage->value, storage->context);
  }
}

StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage, LTCCalibrationLineData *line) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(&(storage->storage)));
  status_ok_or_return(ltc_adc_register_callback(&(storage->storage), prv_callback, storage));

  // Store calibration parameters
  storage->line = line;

  return STATUS_CODE_OK;
}

// Register a callback to run when new data is available
StatusCode ltc_adc_calibration_register_callback(LTCCalibrationStorage *storage,
                                                 LtcAdcCalibrationCallback callback,
                                                 void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->callback = callback;
  storage->context = context;

  return STATUS_CODE_OK;
}
