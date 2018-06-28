#pragma once

// Module for rotary sensor calibration
//
// The goal of this module is to perform a calibration on the
// rotary sensor, obtaining the high and low bound of the rotary
// sensor. Upon acquiring rotary sensor boundaries, it is expected
// to calculate the midpoint and the range of the respective boundaries
// in order to allow for representation of read ADC input in percentage form

#include "steering_wheel.h"

// Contains calibration boundary point information - necessary for calculating
// midpoint and range
typedef struct SteeringWheelCalibrationPointData {
  uint16_t min_reading;
  uint16_t max_reading;
} SteeringWheelCalibrationPointData;

typedef struct SteeringWheelCalibrationSettings {
  ADCChannel adc_channel;
} SteeringWheelCalibrationSettings;

typedef struct SteeringWheelCalibrationStorage {
  SteeringWheelCalibrationPointData data;
  SteeringWheelCalibrationSettings settings;
} SteeringWheelCalibrationStorage;

// Initializes sensor calibration (empties storage data and sets pre-defined settings)
StatusCode steering_wheel_calib_init(SteeringWheelCalibrationStorage *storage,
                                    SteeringWheelCalibrationSettings *settings);

// Clears calibration data before populating it with appropriate key values
// Taken and modified from a calibrated calibration storage
StatusCode steering_wheel_calib_result(SteeringWheelCalibrationStorage *sensor_storage,
                                      SteeringWheelCalibrationData *sensor_calib_data);

// Takes storage in order ot acquire channel from settings
// Takes an independent reading to allow for freedom to set max or min bound
void prv_calc_boundary(ADCChannel read_channel, uint16_t *boundary_reading);
