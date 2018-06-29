#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// The time period between every update of the wheel wheel readings.
#define steering_wheel_UPDATE_PERIOD_MS 10

typedef struct SteeringWheelCalibrationData {
  uint16_t wheel_midpoint;
  uint16_t wheel_range;
  uint16_t max_bound;
  uint16_t min_bound;
  ADCChannel wheel_channel;
} SteeringWheelCalibrationData;

typedef struct SteeringWheelStorage {
  uint16_t wheel_steering_percent;
  SteeringWheelCalibrationData *calibration_data;
} SteeringWheelStorage;

StatusCode steering_wheel_init(SteeringWheelStorage *storage,
                               SteeringWheelCalibrationData *calibration_data);

StatusCode steering_wheel_get_position(SteeringWheelStorage *storage, uint16_t position_reading);
