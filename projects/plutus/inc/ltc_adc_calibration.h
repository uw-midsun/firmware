#pragma once

// Calibration module for the LTC2484 ADC

#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in milliamps
} LTCCalibrationValue;

// User-defined points for two-point calibration
typedef struct {
  LTCCalibrationValue zero_point;
  LTCCalibrationValue max_point;
} LTCCalibrationLineData;

typedef struct {
  LtcAdcStorage storage;
  LTCCalibrationLineData *line;
  LTCCalibrationValue value;
} LTCCalibrationStorage;

// Initialize the calibration module. Requires LTCCalibrationLineData to be filled beforehand
StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage, LTCCalibrationLineData *line);
