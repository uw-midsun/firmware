#pragma once

// Current sense module for the LTC2484 ADC

#include "ltc_adc.h"
#include "status.h"

typedef void (*CurrentSenseCallback)(int32_t current, void *context);

typedef struct {
  int32_t voltage;
  int32_t current;
} CurrentSenseValue;

// User-defined points for two-point calibration
typedef struct {
  CurrentSenseValue zero_point;
  CurrentSenseValue max_point;
} CurrentSenseCalibrationData;

typedef struct {
  LtcAdcStorage adc_storage;
  CurrentSenseCalibrationData data;
  CurrentSenseValue value;
  int32_t offset;
  bool offset_flag;
  CurrentSenseCallback callback;
  void *context;
} CurrentSenseStorage;

// Initialize the current sense module. Requires |data| to be calibrated beforehand.
StatusCode current_sense_init(CurrentSenseStorage *storage, const CurrentSenseCalibrationData data,
                              const LtcAdcSettings *settings);

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context);

// Returns current value in microamps
StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current);

// Call after chip reset to update zero offset value
StatusCode current_sense_zero_reset(CurrentSenseStorage *storage);
