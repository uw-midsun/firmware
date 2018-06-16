#include "current_calibration.h"

#include <string.h>

#include "critical_section.h"
#include "log.h"
#include "wait.h"

static void prv_callback(int32_t *value, void *context) {
  CurrentCalibrationStorage *storage = (CurrentCalibrationStorage *)context;

  if (storage->samples < CURRENT_CALIBRATION_SAMPLES) {
    storage->voltage += *value;
    storage->samples++;
  }
}

StatusCode current_calibration_init(CurrentCalibrationStorage *storage, LtcAdcStorage *adc_storage,
                                    LtcAdcSettings *adc_settings) {
  if (storage == NULL || adc_settings == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  storage->adc_storage = adc_storage;
  storage->settings = adc_settings;
  storage->samples = 0;
  storage->voltage = 0;

  memset(storage->buffer, 0, sizeof(int32_t) * CURRENT_CALIBRATION_OFFSET_WINDOW);
  storage->index = 0;
  storage->num_chip_resets = 1;

  return STATUS_CODE_OK;
}

StatusCode current_calibration_sample_point(CurrentCalibrationStorage *storage,
                                            CurrentSenseValue *point, int32_t current) {
  if (storage == NULL || point == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  status_ok_or_return(ltc_adc_init(storage->adc_storage, storage->settings));
  status_ok_or_return(ltc_adc_register_callback(storage->adc_storage, prv_callback, storage));

  while (storage->samples < CURRENT_CALIBRATION_SAMPLES) {
    wait();
  }

  // Disable callback
  ltc_adc_register_callback(storage->adc_storage, NULL, storage);

  storage->voltage /= CURRENT_CALIBRATION_SAMPLES;

  point->voltage = storage->voltage;
  point->current = current;

  storage->samples = 0;
  storage->voltage = 0;

  // Set the first value in the buffer if we are sampling for the zero point
  if (current == 0 && storage->buffer[0] == 0) {
      storage->buffer[0] = point->voltage;
  }

  return STATUS_CODE_OK;
}

StatusCode current_calibration_zero_reset(CurrentCalibrationStorage *storage,
                                          CurrentSenseValue *zero_point) {
  if (storage == NULL || zero_point == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Obtain new offset voltage
  CurrentSenseValue new_offset = { 0 };
  current_calibration_sample_point(storage, &new_offset, 0);

  // Add new offset to buffer
  storage->index = (storage->index + 1) % CURRENT_CALIBRATION_OFFSET_WINDOW;
  storage->buffer[storage->index] = new_offset.voltage;

  // Update zero point with the result of the new moving average
  zero_point->voltage = (zero_point->voltage * storage->num_chip_resets) + storage->buffer[storage->index];

  if (storage->num_chip_resets < CURRENT_CALIBRATION_OFFSET_WINDOW) {
    storage->num_chip_resets++;
  }

  zero_point->voltage /= storage->num_chip_resets;

  return STATUS_CODE_OK;
}
