#pragma once
// Calibration blob for driver controls
#include "calib.h"
#include "throttle.h"
#include "mech_brake.h"

typedef struct DcCalibBlob {
  ThrottleCalibrationData throttle_calib;
  MechBrakeCalibrationData mech_brake_calib;
} DcCalibBlob;
