#pragma once

#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// The time period between every update of the rotary sensor readings.
#define ROTARY_SENSOR_UPDATE_PERIOD_MS 10

typedef struct RotarySensorCalibrationData {
  uint16_t rotary_midpoint;
  uint16_t rotary_range;
  ADCChannel sensor_channel;
} RotarySensorCalibrationData;

typedef struct RotarySensorStorage {
  uint16_t sensor_steering_percent;
  RotarySensorCalibrationData *calibration_data;
} RotarySensorStorage;

StatusCode rotary_sensor_init(RotarySensorStorage *storage,
                              RotarySensorCalibrationData *calibration_data);
