#pragma once
// Module for the throttle.
// Requires ADS1015, and soft timer to be initialized.
// The module periodically reads the pedal inputs from ADS1015, translates them into
// positions and raises the events that correspond to the pedal's position (braking, coasting, and
// accelerating). It also detects faulty events caused by disconnections or ADS1015 malfunctioning.
// To use the module, init the ADS1015 for the pedal and pass its Ads1015Storage to throttle_init,
// along with the two channels the pedal is connected to. Also pass a ThrottleCalibrationData
// that has been calibrated. This structure holds zone thresholds for determining the state of the
// pedal based on the reading from main channel and two lines that approximate (best fit)
// the voltage-position graph. It is assumed that channel readings hold a linear
// relationship with respect to the pedal's position. The tolerance accounts for possible deviation
// of the data on the second channel when compared to main channel.
// At any time calling throttle_get_position will give the current position of the pedal.
// Note that storage, pedal_ads1015_storage, and calibration_data should persist.

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"

// The time period between every update of the pedal readings.
#define THROTTLE_UPDATE_PERIOD_MS 10

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
  THROTTLE_THRESH_MIN = 0,  //
  THROTTLE_THRESH_MAX,      //
  NUM_THROTTLE_THRESHES
} ThrottleThresh;

// A measure in a 12 bit scale of how far (within a zone) a pedal is pressed.
// I.e. the numerator of a fraction with denominator of 2^12.
typedef uint16_t ThrottleNumerator;

typedef struct ThrottlePosition {
  ThrottleZone zone;
  ThrottleNumerator numerator;
} ThrottlePosition;

// Data that needs to be calibrated.
typedef struct ThrottleCalibrationData {
  // Boundaries of each zone in raw reading format.
  int16_t zone_thresholds_main[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES];
  // The line of best fit described by set of 2 points in raw reading format
  // at the extreme ends for each channel.
  int16_t line_of_best_fit[NUM_THROTTLE_CHANNELS][NUM_THROTTLE_THRESHES];
  int16_t tolerance[NUM_THROTTLE_CHANNELS];
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
