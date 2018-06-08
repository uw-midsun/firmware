#pragma once

// LTC 2484 calibration module

#include "current_sense.h"

// Must be less than 128, or overflow may occur
#define CURRENT_CALIBRATION_SAMPLES 10

typedef struct {
  LtcAdcStorage *adc_storage;
  int32_t voltage;
  volatile uint8_t samples;
} CurrentCalibrationStorage;

// Initialize current calibration
StatusCode current_calibration_init(CurrentCalibrationStorage *storage,
                                            LtcAdcStorage *adc_storage);

// Samples adc readings at specified current in order to obtain data for two-point
// calibration. Function will block until completion. For optimal results, make
// sure the points are as far apart as possible
StatusCode current_calibration_sample_point(CurrentCalibrationStorage *storage,
                                            CurrentSenseValue *point, int32_t current);
