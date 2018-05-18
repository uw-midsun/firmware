#pragma once

// Calibration module for the LTC2484 ADC.
// Requires the ltc_adc to be initialized

// Arbitrary limit on values to track (Maybe keep this small so that it doesn't deviate)
// TODO: adc sampling delay when deciding on final value
#define LTC2484_AVERAGE_WINDOW      5

#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in amps
} LTCCalibrationValue;

typedef struct {
  LtcAdcStorage storage;
  LTCCalibrationValue value;

  // Buffer and index used for moving average
  int32_t buffer[LTC2484_AVERAGE_WINDOW];
  int32_t average;
  uint8_t index;
} LTCCalibrationStorage;

// Initialize the calibration module
StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage);
