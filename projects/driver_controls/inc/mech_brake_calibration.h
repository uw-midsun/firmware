#pragma once
#include "mech_brake.h"
// Calibrates the mechanical brake through user-controlled procedure.
// Requires ADS1015 and soft timers to be initialized.
//
// Module goals:
// * Determine the peak-peak values when the brake is fully pressed and released.
// * Store those values and allow the user to retrieve them.

#define MECH_BRAKE_CALIBRATION_NUM_SAMPLES 1000

typedef enum {
  MECH_BRAKE_CALIBRATION_POINT_UNPRESSED = 0,
  MECH_BRAKE_CALIBRATION_POINT_PRESSED,
  NUM_MECH_CALIBRATION_POINTS
} MechBrakeCalibrationPoint;

typedef struct MechBrakeCalibrationPointData {
  int16_t min_reading;
  int16_t max_reading;
  int16_t average_value;
  volatile uint32_t sample_counter;
} MechBrakeCalibrationPointData;

typedef struct MechBrakeCalibrationStorage {
  MechBrakeSettings settings;
  MechBrakeCalibrationPoint sample_point;
  MechBrakeCalibrationPointData data[NUM_MECH_CALIBRATION_POINTS];
} MechBrakeCalibrationStorage;

// Expects settings.ads1015 to be initialized. Iniializes storage->settings from *settings.
StatusCode mech_brake_calibration_init(MechBrakeCalibrationStorage *storage,
                                       MechBrakeSettings *settings);
// Samples the position at a specified point.
StatusCode mech_brake_sample(MechBrakeCalibrationStorage *storage, MechBrakeCalibrationPoint point);

// Retrieves the calibrated values.
StatusCode mech_brake_get_calib_data(MechBrakeCalibrationStorage *storage,
                                     MechBrakeCalibrationData *calib_data);
                                     