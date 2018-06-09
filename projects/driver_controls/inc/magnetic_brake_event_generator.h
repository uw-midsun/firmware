#pragma once

#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"

typedef struct MagneticBrakeSettings {
  int16_t percentage_threshold;  // value above which the brake_pressed event is raised
  int16_t zero_value;            // lsb value obtained from ads1015 when brake is unpressed
  int16_t hundred_value;         // lsb value obtained from ads105 when brake is pressed
  int16_t min_allowed_range;     // min allowed percentage value, usually 0
  int16_t max_allowed_range;     // max allowed percentage value, (1<<12)
} MagneticBrakeSettings;

typedef struct MagneticCalibrationData {
  int16_t reading;     // value obtained from ads1015 in lsb
  int16_t percentage;  // value calculated from calibration
  Ads1015Storage *storage;
} MagneticCalibrationData;

// this function guides the user of the module through the calibration process to retreive the
// min/max allowed values
StatusCode magnetic_brake_event_generator_init(MagneticCalibrationData *data,
                                               MagneticBrakeSettings *brake_settings,
                                               Ads1015Channel channel);

// this function converts the reading from lsb into percentage from 0 to 2^12
int16_t percentage_converter(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings);
