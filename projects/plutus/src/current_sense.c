#include "current_sense.h"

#include <string.h>

#include "critical_section.h"

static void prv_calculate_current(int32_t *value, void *context) {
  CurrentSenseStorage *storage = (CurrentSenseStorage *)context;

  // Formula for calculating calibrated current. Draws slope between given calibrated
  // points, and uses the result as well as the voltage offset to calculate current
  storage->current_value = storage->data->max_point.current *
                           (*value - storage->data->zero_point.voltage) /
                           (storage->data->max_point.voltage - storage->data->zero_point.voltage);
  if (storage->callback != NULL) {
    storage->callback(storage->current_value, storage->context);
  }
}

StatusCode current_sense_init(CurrentSenseStorage *storage, CurrentSenseCalibrationData *data,
                              LtcAdcStorage *adc_storage, LtcAdcSettings *settings) {
  if (storage == NULL || adc_storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  storage->adc_storage = adc_storage;

  status_ok_or_return(ltc_adc_init(storage->adc_storage, settings));
  status_ok_or_return(
      ltc_adc_register_callback(storage->adc_storage, prv_calculate_current, storage));

  // Store calibration parameters
  storage->data = data;

  // Reset data and callbacks
  storage->current_value = 0;
  storage->callback = NULL;
  storage->context = NULL;

  return STATUS_CODE_OK;
}

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->callback = callback;
  storage->context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current) {
  if (storage == NULL || current == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *current = storage->current_value;

  return STATUS_CODE_OK;
}
