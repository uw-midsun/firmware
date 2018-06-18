#pragma once

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"


typedef struct MechBrakeCalibrationData {
	int16_t zero_value;           // lsb value obtained from ads1015 when brake is unpressed
 	int16_t hundred_value;        // lsb value obtained from ads105 when brake is pressed
} MechBrakeCalibrationData;

typedef struct MechBrakeSettings { 
  Ads1015Storage *ads1015; 
  int16_t min_allowed_range;    // min allowed percentage value, usually 0
  int16_t max_allowed_range;    // max allowed percentage value, (1<<12)
  int16_t percentage_threshold; // value above which the brake_pressed event is raised
  Ads1015Channel channel;
} MechBrakeSettings;

typedef struct MechBrakeStorage {
	int16_t reading;
	int16_t percentage;
    MechBrakeCalibrationData *calibration_data;
    MechBrakeSettings settings;
} MechBrakeStorage;

StatusCode mech_brake_init(MechBrakeStorage *mech_brake_storage, MechBrakeSettings *settings, MechBrakeCalibrationData* calib_data);

int16_t percentage_converter(MechBrakeStorage* storage);
