#pragma once
// Calibrates the throttle through user-controlled procedure.
// Requires ADC and soft timers to be initialized.
//
// This module is expected to be driven through a calibration FSM.
//
// Module goals:
// * Determine primary (higher resolution) channel
// * Find equation for voltage/LSB -> percentage (two-point form)
// * Calculate sync tolerance for secondary (lower resolution) channel
// * Calculate maximum bounds of each zone
//
// Tolerances:
// * Sync tolerance: Maximum difference between calculated percentages of the channels
//                   due to precision errors - note that this only handles steady-state error
//                   and we allow some amount of desync while the pedal is changing rapidly.
// * Sync tolerance safety factor: Additional multiplier to the tolerance value
// * Bounds tolerance: Additional percentage extension to the valid zones of our endpoints to
//                     account for observed over/undervoltage conditions in rapid position changes.
#include "throttle.h"
#include "adc.h"

// Arbitrary amount - seems to work pretty well
#define THROTTLE_CALIBRATION_NUM_SAMPLES 1000

// Initially, we don't know which channel is higher resolution.
// Of course, we could always just require the user to provide that information instead
// of calculating it.
typedef enum {
  THROTTLE_CALIBRATION_CHANNEL_A = 0,
  THROTTLE_CALIBRATION_CHANNEL_B,
  NUM_THROTTLE_CALIBRATION_CHANNELS
} ThrottleCalibrationChannel;

// We use the endpoints to find our two-point equations
typedef enum {
  THROTTLE_CALIBRATION_POINT_FULL_BRAKE = 0,  // Fully released
  THROTTLE_CALIBRATION_POINT_FULL_ACCEL,      // Fully depressed
  NUM_THROTTLE_CALIBRATION_POINTS
} ThrottleCalibrationPoint;

// To account for measurement error/ripple, we aim to build the percentage equations using the
// trendline of our channel measurements. The simple way to do this is to use the middle values
// between each endpoint's peaks. We can use the peak-to-peak value to calculate our sync tolerance.
typedef struct ThrottleCalibrationPointData {
  volatile uint32_t sample_counter;
  int16_t min_reading;
  int16_t max_reading;
} ThrottleCalibrationPointData;

typedef struct ThrottleCalibrationSettings {
  // Channels to consider for throttle calibration
  ADCChannel adc_channel[NUM_THROTTLE_CALIBRATION_CHANNELS];
  // Expects 0-100 for each zone, must sum to 100
  uint8_t zone_percentage[NUM_THROTTLE_ZONES];
  // Multiplier for the sync tolerance
  uint8_t sync_safety_factor;
  // Percentage (0-100) to increase the brake/accel zones outside the measured range
  // TODO(ELEC-382): the proper fix is actually to change the throttle module
  uint8_t bounds_tolerance_percentage;
} ThrottleCalibrationSettings;

typedef struct ThrottleCalibrationStorage {
  // Channels have points
  ThrottleCalibrationPointData data[NUM_THROTTLE_CALIBRATION_CHANNELS]
                                   [NUM_THROTTLE_CALIBRATION_POINTS];
  ThrottleCalibrationSettings settings;
  // Point currently being sampled
  ThrottleCalibrationPoint sample_point;
} ThrottleCalibrationStorage;

// |settings| does not need to persist. |settings.zone_percentage| should add up to 100.
StatusCode throttle_calibration_init(ThrottleCalibrationStorage *storage,
                                     ThrottleCalibrationSettings *settings);

// Samples at the specified point - blocks until completion
StatusCode throttle_calibration_sample(ThrottleCalibrationStorage *storage,
                                       ThrottleCalibrationPoint point);

// Calculates and stores calibration settings
StatusCode throttle_calibration_result(ThrottleCalibrationStorage *storage,
                                       ThrottleCalibrationData *calib_data);
