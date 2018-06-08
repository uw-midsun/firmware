#pragma once

// Current sense module for the LTC2484 ADC

#include "ltc_adc.h"
#include "status.h"

typedef void (*CurrentSenseCallback)(int32_t current, void *context);

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in milliamps
} CurrentSenseValue;

// User-defined points for two-point calibration
typedef struct {
  CurrentSenseValue zero_point;
  CurrentSenseValue max_point;
} CurrentSenseCalibrationData;

typedef struct {
  LtcAdcStorage *adc_storage;
  CurrentSenseCalibrationData *data;
  int32_t value;
  CurrentSenseCallback callback;
  void *context;
} CurrentSenseStorage;

// Initialize the current sense module. Requires CurrentSenseLineData to be calibrated beforehand
StatusCode current_sense_init(CurrentSenseStorage *storage, CurrentSenseCalibrationData *data,
                              LtcAdcStorage *adc_storage, LtcAdcSettings *settings);

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context);

StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current);
