#pragma once

// Module for angle calibration
//
// The goal of this module is to perform a calibration on the
// angle, obtaining the high and low bound of the
// angle. Upon acquiring angle boundaries, it is expected
// to calculate the midpoint and the range of the respective boundaries
// in order to allow for representation of read ADC input in percentage form

// Requires steering_angle.h in order to use steering_angle calibration data struct

#include "steering_angle.h"

// Contains calibration boundary point information - necessary for calculating
// midpoint/range
typedef struct SteeringAngleCalibrationPointData {
  int16_t min_reading;
  int16_t max_reading;
} SteeringAngleCalibrationPointData;

typedef struct SteeringAngleCalibrationStorage {
  SteeringAngleCalibrationPointData data;
  SteeringAngleSettings *settings;
} SteeringAngleCalibrationStorage;

// Initializes angle calibration (empties storage data and sets pre-defined settings)
StatusCode steering_angle_calib_init(SteeringAngleCalibrationStorage *storage,
                                     SteeringAngleSettings *settings);

// Clears calibration data before populating it with appropriate key values
// Taken and modified from a calibrated calibration storage
StatusCode steering_angle_calib_result(SteeringAngleCalibrationStorage *angle_storage,
                                       SteeringAngleCalibrationData *angle_calib_data);

// Takes storage in order ot acquire channel from settings
// Takes an independent reading to allow for freedom to set max or min bound
StatusCode calc_boundary(SteeringAngleCalibrationStorage *storage, int16_t *boundary_reading);