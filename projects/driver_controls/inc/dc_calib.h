#pragma once
// Calibration blob for driver controls
#include "calib.h"
#include "throttle.h"

typedef struct DcCalibBlob {
  ThrottleCalibrationData throttle_calib;
} DcCalibBlob;
