#pragma once

// LTC 2484 calibration module

#include "ltc_current_sense.h"

#define LTC_CALIBRATION_SAMPLES 100

typedef struct {
  LtcAdcStorage *storage;
  int32_t voltage;
  uint8_t samples;
} LTCCalibrationStorage

// Samples adc readings at specified current in order to obtain data for two-point
// calibration. Function will block until completion. For optimal results, make
// sure the points are as far apart as possible 
StatusCode ltc_calibration_sample_point(LTCCalibrationStorage *storage,
                                        LTCCurrentSenseValue *point, int32_t current);
