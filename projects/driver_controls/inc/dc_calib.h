#pragma once
// Calibration blob for driver controls
#include "throttle.h"
#include "calib.h"

typedef struct DcCalibBlob {
  ThrottleCalibrationData throttle_calib;
} DcCalibBlob;
