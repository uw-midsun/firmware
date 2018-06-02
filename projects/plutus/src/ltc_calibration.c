#include "ltc_calibration.h"
#include "critical_section.h"
#include "log.h"
#include "wait.h"

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage*)context;

  storage->voltage += *value;
  storage->samples++;


  LOG_DEBUG("Sample [%d/%d]\n", storage->samples, LTC_CALIBRATION_SAMPLES);
}

StatusCode ltc_calibration_sample_point(LTCCalibrationStorage *storage,
                                        LTCCurrentSenseValue *point, int32_t current) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  ltc_adc_init(&storage->adc_storage);
  ltc_adc_register_callback(&storage->adc_storage, prv_callback, storage);

  storage->samples = 0;
  storage->voltage = 0;

  while (storage->samples < LTC_CALIBRATION_SAMPLES) {
    wait();
  };

  // Disable callback
  ltc_adc_register_callback(&storage->adc_storage, NULL, storage);

  storage->voltage /= LTC_CALIBRATION_SAMPLES;

  point->voltage = storage->voltage;
  point->current = current;

  return STATUS_CODE_OK;
}