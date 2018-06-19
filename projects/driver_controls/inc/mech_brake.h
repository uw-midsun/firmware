#pragma once

// Module for the mechanical brake.
// Requires ADS1015 and soft timers to be initialized.
//
// The module reads brake inputs from ADS1015, then converts those readings into percentage data.
// At the same time, it raises events, INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, and INPUT_EVENT_MECHANICAL_BRAKE_PRESSED
// 
//
// To use the module, init the ADS1015 for the mech brake and pass its Ads1015Storage to mech_brake_init.
// Also pass a MechBrakeCalibrationData that has been calibrated.

// For the percentage conversion, the module receives peak-peak values when the brake is 
// pressed and released, then correlates that data to percentage values and finds a linear equation between the LSB input and percentage.

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
  int16_t min_allowed_range;     // min allowed percentage value, usually 0
  int16_t max_allowed_range;     // max allowed percentage value, (1<<12)
  int16_t percentage_threshold;  // value above which the brake_pressed event is raised
  Ads1015Channel channel;
} MechBrakeSettings;

typedef struct MechBrakeStorage {
  int16_t reading;
  int16_t percentage;
  MechBrakeCalibrationData *calibration_data;
  MechBrakeSettings settings;
} MechBrakeStorage;

StatusCode mech_brake_init(MechBrakeStorage *mech_brake_storage, MechBrakeSettings *settings,
                           MechBrakeCalibrationData *calib_data);

