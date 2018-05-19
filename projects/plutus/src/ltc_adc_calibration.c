#include "ltc_adc_calibration.h"

#include <string.h>

#define LTC2484_OFFSET_MICROVOLTS   1000000

// Start with values obtained from datasheet and update for each sample
static uint8_t s_current_sense_gain = 100;
static uint8_t s_shunt_resistance = 100;

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  // Correct for voltage offset
  storage->value.voltage = *value - LTC2484_OFFSET_MICROVOLTS;

  // Use calibrated voltage value to obtain current calibration_value
  storage->value.current = storage->value.voltage / (s_current_sense_gain * s_shunt_resistance);

  // Update constants based on calculated values
  uint8_t temp = s_current_sense_gain;

  s_current_sense_gain = storage->value.voltage / (storage->value.current * s_shunt_resistance);
  s_shunt_resistance = storage->value.voltage / (storage->value.current * temp);
}

StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(&(storage->storage)));
  status_ok_or_return(ltc_adc_register_callback(&(storage->storage), prv_callback, storage));

  memset(storage->buffer, 0, LTC2484_AVERAGE_WINDOW);
  storage->index = 0;
  storage->average = 0;

  return STATUS_CODE_OK;
}
