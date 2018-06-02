#pragma once

// LTC 2484 calibration module

#include "current_sense.h"

#define CURRENT_CALIBRATION_SAMPLES 10

typedef struct {
  LtcAdcStorage *adc_storage;
  int32_t voltage;
  volatile uint8_t samples;
} CurrentCalibrationStorage;

// Samples adc readings at specified current in order to obtain data for two-point
// calibration. Function will block until completion. For optimal results, make
// sure the points are as far apart as possible
StatusCode current_calibration_sample_point(CurrentCalibrationStorage *storage,
                                            CurrentSenseValue *point, int32_t current);
