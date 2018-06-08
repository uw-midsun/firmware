#include "current_sense.h"

#include <string.h>

#include "critical_section.h"

static void prv_calculate_current(int32_t *value, void *context) {
  CurrentSenseStorage *storage = (CurrentSenseStorage *)context;

  // Correct for voltage offset
  storage->value.voltage = *value - storage->line->zero_point.voltage;

  // Formula for calculating calibrated current. Draws slope between given calibrated
  // points, and uses the result as well as the voltage offset to calculate current
  storage->value.current = storage->line->max_point.current *
                           (*value - storage->line->zero_point.voltage) /
                           (storage->line->max_point.voltage - storage->line->zero_point.voltage);

  if (storage->callback != NULL) {
    storage->callback(storage->value.current, storage->context);
  }
}

StatusCode current_sense_init(CurrentSenseStorage *storage, CurrentSenseCalibrationData *line,
                              LtcAdcStorage *adc_storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  memset(storage, 0, sizeof(CurrentSenseStorage));

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(adc_storage));
  status_ok_or_return(ltc_adc_register_callback(adc_storage, prv_calculate_current, storage));

  // Store calibration parameters
  storage->adc_storage = adc_storage;
  storage->line = line;

  // Reset data and callbacks
  storage->value = (CurrentSenseValue){ 0 };
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
