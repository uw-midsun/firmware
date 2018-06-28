#pragma once
// Calibration module for the photosensor
// Requires adc, soft timer and gpios to be initialized before module can be run.
// Must call init first and then calibrate upper and lower bounds of photo sensor.
// DriverDisplayBrightnessSettings retrieved from driver_display_config same as brightness module.

#include "driver_display_brightness.h"

// The period indicates the amount of time in ms between samples
#define DRIVER_DISPLAY_CALIBRATION_PERIOD_MS 10
#define DRIVER_DISPLAY_CALIBRATION_SAMPLE_SIZE 1000

typedef enum {
  DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND = 0,
  DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND,
  NUM_DRIVER_DISPLAY_CALIBRATION_BOUNDS
} DriverDisplayCalibrationBounds;

typedef struct DriverDisplayCalibrationStorage {
  ADCChannel adc_channel;
  DriverDisplayBrightnessCalibrationData *data;
  volatile bool reading_ok_flag;
  volatile uint64_t sample_sum;
  volatile uint16_t sample_count;
} DriverDisplayCalibrationStorage;

// Initializes the calibration module
// GPIOs and ADC must be initialized beforehand.
StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data,
                                           DriverDisplayCalibrationStorage *storage);

// Calibrates the upper/lower bound by averaging ADC output values for a period of time
// (DRIVER_DISPLAY_CALIBRATION_PERIOD_S) Module init must be called first before running this
// function. Within a period of time, expose the photodiode to maximum brightness levels.
// The bound parameter specifies which bound to adjust
StatusCode driver_display_calibration_bounds(DriverDisplayCalibrationStorage *storage,
                                             DriverDisplayCalibrationBounds bound);
