#include "calibration.h"

#define LTC2484_OFFSET_MILLIVOLTS 100
#define LTC2484_SHUNT_RESISTOR ???

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  // TODO: Calibrate voltage here
  storage->value.voltage = *value - LTC2484_OFFSET_MILLIVOLTS;

  // TODO: Calculate current here
  storage->value.current = storage->value.voltage / LTC2484_SHUNT_RESISTOR;
}

StatusCode calibration_value(LTCCalibrationStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(&(storage->storage)));
  status_ok_or_return(ltc_adc_register_callback(&(storage->storage), prv_callback, storage));

  return STATUS_CODE_OK;
}
