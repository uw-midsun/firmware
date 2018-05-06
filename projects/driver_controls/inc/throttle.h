#pragma once
// Module for the throttle.
// Requires ADS1015, and soft timer to be initialized.
//
// The module periodically reads pedal inputs from ADS1015, translates and stores them as positions.
// At the same time, it raises events, INPUT_EVENT_PEDAL_BRAKE, _COAST, and _ACCEL which would
// match the current position, and for the event's data field, a numerator to describe the exact
// position of the pedal within current zone. (The numerator for coast zone is irrelevant).
// The module also detects faulty events such as disconnections or ADS1015 malfunctioning which
// could result in reading invalid numbers. INPUT_EVENT_PEDAL_FAULT would be raised in such cases.
//
// To use the module, init the ADS1015 for the pedal and pass its Ads1015Storage to throttle_init.
// Also pass a ThrottleCalibrationData that has been calibrated.
// This structure holds zone thresholds for determining the state of the pedal
// based on the reading from main channel, and two lines that approximate the voltage-position
// graph for each channel. The main line is the main source for obtaining the position, and the
// second line along with a tolerance is used to check if the readings agree. It is assumed that
// channel readings hold a linear relationship with respect to the pedal's position.
// Look at "Math behind Throttle Module" on Confluence to see how the logic exactly works.
//
// At any time calling throttle_get_position will give the current position of the pedal.
// Note that storage, pedal_ads1015_storage, and calibration_data should persist.

#include <stdint.h>
#include "ads1015.h"
#include "soft_timer.h"
#include "status.h"

// The time period between every update of the pedal readings.
#define THROTTLE_UPDATE_PERIOD_MS 10
// Arbitrary maximum desync counter - this is used to handle rapid changes in pedal position
// Currently ~100ms
#define THROTTLE_MAX_DESYNC_COUNT 10

// The range used for pedal's position in within a zone or the whole range.
#define THROTTLE_DENOMINATOR (1 << 12)

// The measure of how far (within a zone or the full range) the pedal is pressed.
// I.e. the numerator of a fraction with denominator THROTTLE_DENOMINATOR.
typedef uint16_t ThrottleNumerator;

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

typedef struct ThrottlePosition {
  ThrottleZone zone;
  ThrottleNumerator numerator;
} ThrottlePosition;

typedef struct ThrottleZoneThreshold {
  int16_t min;
  int16_t max;
} ThrottleZoneThreshold;

// Models a voltage-position line using the readings at extreme ends.
typedef struct ThrottleLine {
  int16_t full_brake_reading;     // Reading for pedal not pressed.
  int16_t full_throttle_reading;  // Reading for pedal fully pressed.
} ThrottleLine;

// Data that needs to be calibrated.
typedef struct ThrottleCalibrationData {
  // Boundaries of each zone in raw reading format.
  ThrottleZoneThreshold zone_thresholds_main[NUM_THROTTLE_ZONES];
  // Lines that describe voltage-position graph for channels.
  ThrottleLine line[NUM_THROTTLE_CHANNELS];
  // Tolerance can be thought of as 1/2 of width of a band around the secondary line,
  // which describes a range of possible "secondary readings" for every single "main reading".
  int16_t tolerance;
  Ads1015Channel channel_main;
  Ads1015Channel channel_secondary;
} ThrottleCalibrationData;

typedef struct ThrottleStorage {
  Ads1015Storage *pedal_ads1015_storage;
  ThrottleCalibrationData *calibration_data;
  ThrottlePosition position;
  uint32_t desync_counter;
  bool reading_updated_flag;
  bool reading_ok_flag;
} ThrottleStorage;

// Initializes the throttle and sets calibration data.
// Ads1015Storage *pedal_ads1015_storage should be initialized in ads1015_init beforehand.
StatusCode throttle_init(ThrottleStorage *storage, ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage);

// Gets the current position of the pedal (writes to ThrottlePosition *position).
StatusCode throttle_get_position(ThrottleStorage *storage, ThrottlePosition *position);
