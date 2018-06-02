#pragma once

// Current sense module for the LTC2484 ADC

#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in milliamps
} CurrentSenseValue;

typedef void (*CurrentSenseCallback)(CurrentSenseValue *value, void *context);

// User-defined points for two-point calibration
typedef struct {
  CurrentSenseValue zero_point;
  CurrentSenseValue max_point;
} CurrentSenseLineData;

typedef struct {
  LtcAdcStorage *adc_storage;
  CurrentSenseLineData *line;
  CurrentSenseValue value;
  CurrentSenseCallback callback;
  void *context;
} CurrentSenseStorage;

// Initialize the current sense module. Requires adc_storage to be initialized and
// CurrentSenseLineData to be calibrated beforehand
StatusCode current_sense_init(CurrentSenseStorage *storage, CurrentSenseLineData *line,
                              LtcAdcStorage *adc_storage);

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback, void *context);
