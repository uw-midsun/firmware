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
  LtcAdcStorage adc_storage;
  CurrentSenseCalibrationData *data;
  CurrentSenseValue value;
  int32_t offset;
  CurrentSenseCallback callback;
  void *context;
} CurrentSenseStorage;

// Initialize the current sense module. Requires |data| to be calibrated beforehand.
// LtcAdcStorage does not need to be initialized
StatusCode current_sense_init(CurrentSenseStorage *storage, CurrentSenseCalibrationData *data,
                              LtcAdcSettings *settings);

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context);

// Return current in millamps
StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current);

// Because the zero point for the chip changes on reset, this function can be called after reset to
// adjust the data points to make sure they maintain their linear relationship
StatusCode current_sense_zero_reset(CurrentSenseStorage *storage);
