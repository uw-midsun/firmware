#pragma once
// Module for the throttle.
// Requires ADS1015 and soft timers to be initialized.
//
// Note: Also requires calibration routines to have been run.
//
// The module periodically reads the position of the throttle via the ADS1015,
// and scales those readings into a numerator value corresponding to a
// denominator of THROTTLE_DENOMINATOR.
//
// At the same time, it raises events when the throttle enters the various
// pedal zones:
//
//    * INPUT_EVENT_PEDAL_BRAKE
//    * INPUT_EVENT_COAST
//    * INPUT_EVENT_ACCEL
//
// These events contain the numerator value that corresponds to the input
// reading of the ADC in the data field. This allows us to trigger FSM changes
// in the application code.
//
// Note: The position when we are in the Coast zone is ignored, but kept for
// telemetry purposes.
//
// The module also detects faulty events such as bad sensor readings, which
// could potentially be a safety issue. A fault event would be raised for FSMs
// to process in such cases.
//
//
// At any time calling throttle_get_position will give the current position of the pedal.
// Note that storage, pedal_ads1015_storage, and calibration_data should persist.

#include <stdint.h>

#include "ads1015.h"
#include "exported_enums.h"
#include "soft_timer.h"
#include "status.h"

// The time period between every update of the pedal readings.
#define THROTTLE_UPDATE_PERIOD_MS 10

// The range used for pedal's position in within a zone or the whole range.
#define THROTTLE_DENOMINATOR EE_DRIVE_OUTPUT_DENOMINATOR

// The measure of how far (within a zone or the full range) the pedal is
// pressed.
//
// This is the position numerator of a fraction with denominator
// THROTTLE_DENOMINATOR.
typedef uint16_t ThrottleNumerator;

// Throttle Zones that the throttle can be in
typedef enum {
  THROTTLE_ZONE_BRAKE = 0,
  THROTTLE_ZONE_COAST,
  THROTTLE_ZONE_ACCEL,
  NUM_THROTTLE_ZONES
} ThrottleZone;

// Throttle Channels
//
// We use the second channel for additional redundancy in case the throttle
// reading from one channel goes bad.
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
  // Reading for when the pedal is fully released.
  int16_t full_brake_reading;
  // Reading for when the pedal is fully pressed.
  int16_t full_accel_reading;
} ThrottleLine;

// Data that needs to be calibrated.
typedef struct ThrottleCalibrationData {
  // Boundaries of each zone in raw reading format.
  ThrottleZoneThreshold zone_thresholds_main[NUM_THROTTLE_ZONES];
  // Lines that describe voltage-position graph for channels.
  ThrottleLine line[NUM_THROTTLE_CHANNELS];
  // Tolerance is essentially the amount of hystereis that is acceptable in the
  // second channel when looking at the secondary reading's data in comparison
  // to the expected secondary reading if the main channel's reading were good.
  //
  // It is assumed that the channel readings hold a linear relationship with
  // respect to the pedal's position. The main channel is used as the master
  // source, and the secondary line is used to verify the reading.
  //
  // Look at "Math behind Throttle Module" on Confluence to see how the logic
  // exactly works.
  int16_t tolerance;
  // Primary ADC channel.
  Ads1015Channel channel_main;
  // Secondary ADC channel used to provide redundancy.
  Ads1015Channel channel_secondary;
} ThrottleCalibrationData;

typedef struct ThrottleStorage {
  Ads1015Storage *pedal_ads1015_storage;
  ThrottleCalibrationData *calibration_data;
  // Current throttle position.
  ThrottlePosition position;
  // Indicates whether there is a fault or not
  bool reading_ok_flag;
} ThrottleStorage;

// Initializes the throttle and stores the calibration data
StatusCode throttle_init(ThrottleStorage *storage, ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage);

// Gets the current position of the pedal (writes to ThrottlePosition *position).
StatusCode throttle_get_position(ThrottleStorage *storage, ThrottlePosition *position);

// Returns a pointer to the global throttle storage.
//
// Note: This only exists because our FSMs already use their context pointers
// for event arbiters. We could potentially remove this by wrapping this in a
// context that the FSM stores instead when initialized.
ThrottleStorage *throttle_global(void);
