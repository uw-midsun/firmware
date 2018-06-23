#pragma once

// Current sense module for the LTC2484 ADC.
// Requires gpio, interrupts, and soft timers to be initialized

#include "ltc_adc.h"
#include "status.h"

// If the adc has not produced valid data (i.e. timeout), then |current| will be a null pointer
typedef void (*CurrentSenseCallback)(int32_t *current, void *context);

typedef struct {
  int32_t voltage;
  int32_t current;
} CurrentSenseValue;

// Line data for two-point calibration
typedef struct {
  CurrentSenseValue zero_point;
  CurrentSenseValue max_point;
} CurrentSenseCalibrationData;

typedef struct {
  LtcAdcStorage adc_storage;
  CurrentSenseCalibrationData data;
  CurrentSenseValue value;
  bool data_valid;
  int32_t offset;
  bool offset_pending;
  CurrentSenseCallback callback;
  void *context;
} CurrentSenseStorage;

// Initialize the current sense module. Requires |data| to be calibrated beforehand.
StatusCode current_sense_init(CurrentSenseStorage *storage,
                              const CurrentSenseCalibrationData *data,
                              const LtcAdcSettings *settings);

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context);

// Returns current value in microamps
StatusCode current_sense_get_value(CurrentSenseStorage *storage, bool *data_valid,
                                   int32_t *current);

// Call after chip reset to update zero offset value
StatusCode current_sense_zero_reset(CurrentSenseStorage *storage);
