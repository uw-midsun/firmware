#pragma once
// Module for the throttle.
// Requires Ads1015, and soft timer to be initialized.
// The module periodically reads the pedal inputs from ADS1015, translates them into
// positions and raises the events that correspond to the pedal's position (braking, coasting, and
// accelerating). It also checks if the readings are stale (in case of a disconnection for
// example) and could raise a timeout event.
// To use the module, init the ADS1015 for the pedal and pass its Ads1015Storage to throttle_init,
// along with the two channels the pedal is connected to. Also pass a ThrottleCalibrationData
// initialized with desired values. At any time calling throttle_get_position will give the current
// position of the pedal.
// Note that storage, pedal_ads1015_storage, and calibration_data should persist.

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"

// The time period between every update of the pedal readings.
#define THROTTLE_UPDATE_PERIOD_MS 50

typedef enum {
  THROTTLE_ZONE_BRAKE = 0,
  THROTTLE_ZONE_COAST,
  THROTTLE_ZONE_ACCEL,
  NUM_THROTTLE_ZONES
} ThrottleZone;

typedef enum {
  THROTTLE_CHANNEL_MAIN = 0,
  THROTTLE_CHANNEL_SECONDARY,
  NUM_THROTTLE_CHANNELS
} ThrottleChannel;

typedef enum {
  THROTTLE_THRESH_MIN = 0,  
  THROTTLE_THRESH_MAX,      
  NUM_THROTTLE_THRESHES
} ThrottleThresh;

// A measure in a 12 bit scale of how far within a zone a pedal is pressed.
// I.e. the numerator of a fraction with denominator of 2^12.
typedef uint16_t ThrottleNumerator;

typedef struct ThrottlePosition {
  ThrottleZone zone;
  ThrottleNumerator numerator;
} ThrottlePosition;

// Data that needs to be calibrated.
typedef struct ThrottleCalibrationData {
  int16_t zone_thresholds[NUM_THROTTLE_CHANNELS][NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES];
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

// Initializes the throttle and sets calibration data.
// Ads1015Storage *pedal_ads1015_storage should be initialized in ads1015_init beforehand.
StatusCode throttle_init(ThrottleStorage *storage, ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage, Ads1015Channel channel_main,
                         Ads1015Channel channel_secondary);

// Gets the current position of the pedal (writes to ThrottlePosition *position).
StatusCode throttle_get_position(ThrottleStorage *storage, ThrottlePosition *position);
