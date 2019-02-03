#pragma once
// Calibration blob for driver controls
#include "calib.h"
#include "mech_brake.h"
#include "throttle.h"

typedef struct DcCalibBlob {
  ThrottleCalibrationData throttle_calib;
  MechBrakeCalibrationData mech_brake_calib;
} DcCalibBlob;