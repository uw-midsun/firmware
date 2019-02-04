#pragma once

// Module for steering angle angle percentage reading
// Requires ADS1015 to be initialized
//
// The goal of this module is to convert the analog reading of a rotary
// sensor to an accurate percentage output. This will be accomplished by
// using the ADC to convert the sensor's analog input to digital input, and
// using calibrated data from the steering angle calibration module to accurately
// convert digital input (on a scale of 0 to 2047) to a scale of (-1024 to 1024)
// ,which is converted to a percentage scale of (-100% to 100%),
// where -100% is the counterclockwise-most position of the angle, and
// 100% is the clockwise-most position of the angle.

// Note that to accomplish the digital to percentage conversion, a calibrated
// SteeringAngleCalibrationData is required

#include <stdbool.h>
#include <stdio.h>

#include "ads1015.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Calibrated data from the steering angle calibration module
typedef struct SteeringAngleCalibrationData {
  uint16_t angle_midpoint;
  int16_t max_bound;
  int16_t min_bound;
  uint16_t tolerance_percentage;
} SteeringAngleCalibrationData;

typedef struct SteeringAngleSettings {
  Ads1015Storage *ads1015;
  Ads1015Channel adc_channel;
} SteeringAngleSettings;

typedef struct SteeringAngleStorage {
  Ads1015Storage *ads1015;
  Ads1015Channel adc_channel;
  int16_t angle_steering_percent;
  SteeringAngleCalibrationData *calibration_data;
} SteeringAngleStorage;

// stores initialized calibration data into SteeringAngleStorage's SteeringAngleCalibrationData
StatusCode steering_angle_init(SteeringAngleStorage *storage,
                               SteeringAngleCalibrationData *calibration_data,
                               SteeringAngleSettings *settings);

// Reads analog input, converts analog input to digital, and then converts
// digital to percentage and stores value into SteeringAngleStorage
StatusCode steering_angle_get_position(SteeringAngleStorage *storage);

StatusCode steering_angle_get_position_test(SteeringAngleStorage *steering_angle_storage,
                                            int16_t reading);
                                            