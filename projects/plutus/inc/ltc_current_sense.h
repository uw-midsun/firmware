#pragma once

// Current sense module for the LTC2484 ADC

#include "ltc_adc.h"
#include "status.h"

typedef struct {
  int32_t voltage;  // Voltage in microvolts
  int32_t current;  // Voltage in milliamps
} LTCCurrentSenseValue;

typedef void (*LtcCurrentSenseCallback)(LTCCurrentSenseValue *value, void *context);

// User-defined points for two-point calibration
typedef struct {
  LTCCurrentSenseValue zero_point;
  LTCCurrentSenseValue max_point;
} LTCCurrentSenseLineData;

typedef struct {
  LtcAdcStorage *adc_storage;
  LTCCurrentSenseLineData *line;
  LTCCurrentSenseValue value;
  LtcCurrentSenseCallback callback;
  void *context;
} LTCCurrentSenseStorage;

// Initialize the current sense module. Requires adc_storage to be initialized and
// LTCCurrentSenseLineData to be calibrated beforehand
StatusCode ltc_current_sense_init(LTCCurrentSenseStorage *storage, LTCCurrentSenseLineData *line,
                                  LtcAdcStorage *adc_storage);

// Register a callback to run when new data is available
StatusCode ltc_current_sense_register_callback(LTCCurrentSenseStorage *storage,
                                                 LtcCurrentSenseCallback callback, void *context);
