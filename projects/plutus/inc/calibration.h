#pragma once

// Calibration module for the LTC2484 ADC.
// Requires the ltc_adc to be initialized

// TODO: Rename module
#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;
  int32_t current;
} LTCCalibrationValue;

typedef struct {
  LtcAdcStorage storage;
  LTCCalibrationValue value;
} LTCCalibrationStorage;

// Initialize the calibration module 
StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage);
