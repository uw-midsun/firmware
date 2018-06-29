#pragma once

// Module for steering wheel angle percentage reading
//
// The goal of this module is to convert the analog reading of a rotary
// sensor to an accurate percentage output. This will be accomplished by
// using the ADC to convert the sensor's analog input to digital input, and
// using calibrated data from the steering wheel calibration module to accurately
// convert digital input (on a scale of 0 to 4095) to a scale of (-100% to 100%),
// where -100% is the counterclockwise-most position of the wheel, and
// 100% is the clockwise-most position of the wheel.
//
// Requires steering wheel calibration module

#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Calibrated data from the steering wheel calibration module
typedef struct SteeringWheelCalibrationData {
  uint16_t wheel_midpoint;
  uint16_t wheel_range;
  uint16_t max_bound;
  uint16_t min_bound;
  ADCChannel wheel_channel;
} SteeringWheelCalibrationData;

typedef struct SteeringWheelStorage {
  int16_t wheel_steering_percent;
  SteeringWheelCalibrationData *calibration_data;
} SteeringWheelStorage;

StatusCode steering_wheel_init(SteeringWheelStorage *storage,
                               SteeringWheelCalibrationData *calibration_data);

StatusCode steering_wheel_get_position(SteeringWheelStorage *storage);
