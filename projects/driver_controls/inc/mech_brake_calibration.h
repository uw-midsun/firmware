#pragma once
#include "mech_brake.h"
// Calibrates the mechanical brake through user-controlled procedure.
// Requires ADS1015 and soft timers to be initialized.
//
// Module goals:
// * Determine the peak-peak values when the brake is fully pressed and released
// * Find equation for lsb/percentage
// * Raise events based on data read from sensor
//
//
// Tolerances:
// percentage tolerance that determines when to raise an event

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
