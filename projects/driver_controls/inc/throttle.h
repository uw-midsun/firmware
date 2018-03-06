#pragma once
// Module for the throttle.
// Requires Ads1015, and soft timer to be initialized.
// The module periodically reads the pedal inputs from ADS1015, translates them into
// positions and raises the events that correspond to the pedal's position (braking, coasting, and
// accelerating). It also checks if the readings are stale (in case of a disconnection for
// example) and could raise a timeout event.
// To use the module, init the ADS1015 for the pedal and pass its Ads1015Storage to throttle_init,
// along with the two channels the pedal is connected to. At any time calling throttle_get_position
// will give the current position of the pedal.
// * throttle_storage should persist across functions.
// ** The throttle should be calibrated beforehand. The throttle_set_calibration_data function
// provides a temporary method for doing so manually before a calibration routine is implemented.

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"

typedef enum {
  THROTTLE_ZONE_BRAKE = 0,
  THROTTLE_ZONE_COAST,
  THROTTLE_ZONE_ACCEL,
  NUM_THROTTLE_ZONES
} ThrottleZone;

// A measure in a 12 bit scale of how far within a zone a pedal is pressed.
typedef uint16_t ThrottleFraction;

typedef struct ThrottlePosition {
  ThrottleZone zone;
  ThrottleFraction fraction;
} ThrottlePosition;

// Data that needs to be calibrated.
typedef struct ThrottleCalibrationData {
  int16_t main_bottom_thresh;
  int16_t main_brake_thresh;
  int16_t main_coast_thresh;
  int16_t main_accel_thresh;
  int16_t secondary_bottom_thresh;
  int16_t secondary_accel_thresh;
  int16_t channel_readings_tolerance;
} ThrottleCalibrationData;

typedef struct ThrottleStorage {
  Ads1015Storage *pedal_ads1015_storage;
  bool reading_updated_flag;
  bool reading_ok_flag;
  Ads1015Channel channel_main;
  Ads1015Channel channel_secondary;
  SoftTimerID raise_event_timer_id;
  ThrottlePosition position;
  ThrottleCalibrationData *calibration_data;
} ThrottleStorage;

// Initializes the throttle.
// Ads1015Storage *pedal_ads1015_storage should be initialized in ads1015_init beforehand.
StatusCode throttle_init(ThrottleStorage *throttle_storage, Ads1015Storage *pedal_ads1015_storage,
                         Ads1015Channel channel_0, Ads1015Channel channel_1);

// Gets the current position of the pedal (writes to ThrottlePosition *position).
StatusCode throttle_get_position(ThrottleStorage *throttle_storage, ThrottlePosition *position);

// A temporary method for setting the calibration data.
StatusCode throttle_set_calibration_data(ThrottleStorage *throttle_storage,
                                         ThrottleCalibrationData *calibration_data);
