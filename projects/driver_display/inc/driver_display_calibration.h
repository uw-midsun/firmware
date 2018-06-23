#pragma once

#include "driver_display_brightness.h"

#define DRIVER_DISPLAY_CALIBRATION_PERIOD_S 5

StatusCode driver_display_calibration_init(const DriverDisplayBrightnessSettings *settings,
                                           DriverDisplayBrightnessCalibrationData *data);
