#include "ltc_calibration.h"

// TODO: Remove stdio.h
#include <stdio.h>

#define LTC2484_OFFSET_MICROVOLTS 100000
#define LTC2484_SHUNT_RESISTOR 100

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  // Correct for voltage offset
  storage->value.voltage = *value - LTC2484_OFFSET_MICROVOLTS;

  // Use calibrated voltage value to obtain current calibration_value
  storage->value.current = storage->value.voltage / LTC2484_SHUNT_RESISTOR;
}

StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(&(storage->storage)));
  status_ok_or_return(ltc_adc_register_callback(&(storage->storage), prv_callback, storage));

  return STATUS_CODE_OK;
}
