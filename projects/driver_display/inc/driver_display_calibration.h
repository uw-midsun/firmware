#pragma once
// Calibration module for the photosensor
// Requires adc, soft timer and gpios to be initialized before module can be run.
// Must call init first and then calibrate upper and lower bounds of photo sensor.
// DriverDisplayBrightnessSettings retrieved from driver_display_config same as brightness module.

#include "driver_display_brightness.h"

#define DRIVER_DISPLAY_CALIBRATION_PERIOD_S 3

typedef struct DriverDisplayCalibrationStorage {
  ADCChannel adc_channel;
  DriverDisplayBrightnessCalibrationData *data;
} DriverDisplayCalibrationStorage;

// Initializes the calibration module
// GPIOs and ADC must be initialized beforehand.
StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data,
                                           DriverDisplayCalibrationStorage *storage);

// Calibrates the upper bound by averaging ADC output values for a period of time
// (DRIVER_DISPLAY_CALIBRATION_PERIOD_S) Module init must be called first before running this
// function. Within a period of time, expose the photodiode to maximum brightness levels.
StatusCode driver_display_calibration_upper_bound(DriverDisplayCalibrationStorage *storage);

// Calibrates the lower bound by averaging ADC output values for a period of time
// (DRIVER_DISPLAY_CALIBRATION_PERIOD_S) Module init must be called first before running this
// function. Within a period of time, expose the photodiode to minimum brightness levels.
StatusCode driver_display_calibration_lower_bound(DriverDisplayCalibrationStorage *storage);
