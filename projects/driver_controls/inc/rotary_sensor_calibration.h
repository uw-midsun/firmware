#pragma once

// Module for rotary sensor calibration
//
// The goal of this module is to perform a calibration on the
// rotary sensor, obtaining the high and low bound of the rotary
// sensor. Upon acquiring rotary sensor boundaries, it is expected
// to calculate the midpoint and the range of the respective boundaries
// in order to allow for representation of read ADC input in percentage form

#include "rotary_sensor.h"

// Contains calibration boundary point information - necessary for calculating
// midpoint and range
typedef struct RotarySensorCalibrationPointData {
  uint16_t min_reading;
  uint16_t max_reading;
} RotarySensorCalibrationPointData;

typedef struct RotarySensorCalibrationSettings {
  ADCChannel adc_channel;
} RotarySensorCalibrationSettings;

typedef struct RotarySensorCalibrationStorage {
  RotarySensorCalibrationPointData data;
  RotarySensorCalibrationSettings settings;
} RotarySensorCalibrationStorage;

// Initializes sensor calibration (empties storage data and sets pre-defined settings)
StatusCode rotary_sensor_calib_init(RotarySensorCalibrationStorage *storage,
                                    RotarySensorCalibrationSettings *settings);

// Clears calibration data before populating it with appropriate key values
// Taken and modified from a calibrated calibration storage
StatusCode rotary_sensor_calib_result(RotarySensorCalibrationStorage *sensor_storage,
                                      RotarySensorCalibrationData *sensor_calib_data);

// Takes storage in order ot acquire channel from settings
// Takes an independent reading to allow for freedom to set max or min bound
void prv_calc_boundary(ADCChannel read_channel, uint16_t *boundary_reading);
