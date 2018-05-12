#pragma once

// Calibration module for the LTC2484 ADC.
// Requires the ltc_adc to be initialized

#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in amps
} LTCCalibrationValue;

typedef struct {
  LtcAdcStorage storage;
  LTCCalibrationValue value;
} LTCCalibrationStorage;

// Initialize the calibration module
StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage);
