#pragma once
#include "mech_brake.h"

#define MECH_BRAKE_CALIBRATION_NUM_SAMPLES 1000

typedef enum {
  MECH_BRAKE_CALIBRATION_POINT_UNPRESSED = 0,
  MECH_BRAKE_CALIBRATION_POINT_PRESSED,
  NUM_MECH_CALIBRATION_POINTS
} MechBrakeCalibrationPoint;

typedef struct MechBrakeCalibrationPointData {
  int16_t min_reading;
  int16_t max_reading;
  volatile uint32_t sample_counter;
} MechBrakeCalibrationPointData;

typedef struct MechBrakeCalibrationStorage {
  MechBrakeSettings settings;

  MechBrakeCalibrationPoint sample_point;

  MechBrakeCalibrationPointData data[NUM_MECH_CALIBRATION_POINTS];
} MechBrakeCalibrationStorage;

StatusCode mech_brake_calibration_init(MechBrakeCalibrationStorage *storage,
                                       MechBrakeSettings *settings);

StatusCode mech_brake_sample(MechBrakeCalibrationStorage *storage, MechBrakeCalibrationPoint point);

StatusCode mech_brake_calibration_result(MechBrakeCalibrationStorage *storage,
                                         MechBrakeCalibrationData *calib_data);
