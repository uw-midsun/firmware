#pragma once

// Module for the mechanical brake.
// Requires ADS1015 and soft timers to be initialized.
//
// The module reads brake inputs from ADS1015, then converts those readings into percentage data.
// At the same time, it raises events, INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, and
// INPUT_EVENT_MECHANICAL_BRAKE_PRESSED. These events contain a percentage value that corresponds
// to the input reading of the adc.
//
// For the percentage conversion, the module receives peak-peak values when the brake is
// pressed and released, then correlates that data to percentage values and finds a linear equation
// between the LSB input and percentage.

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"

typedef struct MechBrakeCalibrationData {
  int16_t zero_value;
  int16_t hundred_value;
} MechBrakeCalibrationData;

typedef struct MechBrakeSettings {
  Ads1015Storage *ads1015;
  int16_t brake_pressed_threshold;  // value above which the brake_pressed event is raised and below
                                    // which the brake_unpressed event is raised
  int16_t tolerance;
  Ads1015Channel channel;
} MechBrakeSettings;

typedef struct MechBrakeStorage {
  MechBrakeCalibrationData *calibration_data;
  MechBrakeSettings settings;
} MechBrakeStorage;

StatusCode mech_brake_init(MechBrakeStorage *mech_brake_storage, MechBrakeSettings *settings,
                           MechBrakeCalibrationData *calib_data);

StatusCode mech_brake_get_percentage(MechBrakeStorage *storage, int16_t *percentage);

