#pragma once

// Module for the mechanical brake.
// Requires ADS1015 and soft timers to be initialized.
//
// The module reads brake inputs from ADS1015, then converts those readings into a numerator value
// with EE_DRIVE_OUTPUT_DENOMINATOR as the denomiator. This numerator value is called the position.
// At the same time, it raises events, INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, and
// INPUT_EVENT_MECHANICAL_BRAKE_PRESSED. These events contain the numerator value that corresponds
// to the input reading of the adc.
//
// For the LSB to position conversion, the module receives peak-peak values when the brake is
// pressed and released, then correlates that data to a position value and generates a linear
// equation between the LSB input and position.

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
  // Value above which the brake_pressed event is raised and below which the brake_unpressed event
  // is raised.
  int16_t brake_pressed_threshold;
  // A tolerance between 0 - 100 for the lower and upper bound of the position.
  int16_t bounds_tolerance;
  Ads1015Channel channel;
} MechBrakeSettings;

typedef struct MechBrakeStorage {
  MechBrakeCalibrationData *calibration_data;
  Ads1015Storage *ads1015;
  Ads1015Channel channel;
  // minimun value of the position based on the tolerance value.
  int16_t lower_bound;
  // maximum value of the position based on the tolerance value.
  int16_t upper_bound;
  int16_t threshold_position;
} MechBrakeStorage;

// Initializes the mech brake by configuring the ADS1015 channel.
// Calculates the upper and lower bound of the position.
StatusCode mech_brake_init(MechBrakeStorage *mech_brake_storage, const MechBrakeSettings *settings,
                           const MechBrakeCalibrationData *calib_data);

// Gets the current position of the mech_brake and writes that value to *position
StatusCode mech_brake_get_position(MechBrakeStorage *storage, int16_t *position);
