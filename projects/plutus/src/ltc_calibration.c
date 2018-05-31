#include "ltc_adc_calibration.h"
#include "wait.h"

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  storage->voltage += value;
  storage->samples++;
}

StatusCode ltc_calibration_sample_point(LTCCalibrationStorage *storage,
                                        LTCCurrentSenseValue *point, int32_t current) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  ltc_adc_init(&storage->storage);
  ltc_adc_register_callback(storage->storage, prv_callback, storage);

  storage->samples = 0;
  storage->voltage = 0;

  while (storage->samples < LTC_CALIBRATION_SAMPLES) {};

  voltage /= LTC_CALIBRATION_SAMPLES;

  point->voltage = voltage;
  point->current = current;

  return STATUS_CODE_OK;
}