#pragma once
// This module provides the means for pedal calibration.
// Calibration has basically 3 stages. First stage is about reading and recording pedal inputs from
// pedal at full brake state. The second stage is the same as first but for full throttle state.
// The last stage is about calculating the results into a ThrottleCalibrationData structure that
// throttle module would understand and validating correctness through user's manual testing of
// the pedal.
//
// The procedure is set up in test_pedal_calibration with the help of a FSM that when switched to a
// state calls the corresponding functions from this module.
// Look at pedal_calibration_fsm module for more information.
#include "ads1015.h"
#include "throttle.h"

typedef enum {
  PEDAL_CALIBRATION_CHANNEL_A = 0,
  PEDAL_CALIBRATION_CHANNEL_B,
  NUM_PEDAL_CALIBRATION_CHANNELS
} PedalCalibrationChannel;

typedef enum {
  PEDAL_CALIBRATION_STATE_FULL_THROTTLE = 0,
  PEDAL_CALIBRATION_STATE_FULL_BRAKE,
  NUM_PEDAL_CALIBRATION_STATES
} PedalCalibrationState;

typedef struct PedalCalibrationRange {
  int16_t min;
  int16_t max;
} PedalCalibrationRange;

typedef struct PedalCalibrationStorage {
  ThrottleCalibrationData *throttle_calibration_data;
  PedalCalibrationRange band[NUM_PEDAL_CALIBRATION_CHANNELS][NUM_PEDAL_CALIBRATION_STATES];
  Ads1015Storage *ads1015_storage;
  Ads1015Channel adc_channel[NUM_PEDAL_CALIBRATION_CHANNELS];
  PedalCalibrationState state;
  uint8_t sample_counter[NUM_PEDAL_CALIBRATION_CHANNELS];
  uint8_t brake_percentage;
  uint8_t coast_percentage;
  uint8_t safety_factor;
} PedalCalibrationStorage;

// Initializes the pedal calibration storage. Ads1015Storage should be initialized.
StatusCode pedal_calibration_init(PedalCalibrationStorage *storage, Ads1015Storage *ads1015_storage,
                                  Ads1015Channel channel_a, Ads1015Channel channel_b,
                                  uint8_t brake_zone_percentage, uint8_t coast_zone_percentage,
                                  uint8_t tolerance_safety_factor);

// Finds and stores max and min readings from pedal in a given state for both channels.
// The pedal should be kept at the given state until the function finishes sampling.
StatusCode pedal_calibration_process_state(PedalCalibrationStorage *storage,
                                           PedalCalibrationState state);

// This should be called only after calling pedal_calibration_process_state.
// It initializes ThrottleCalibrationData based on the read data.
StatusCode pedal_calibration_calculate(PedalCalibrationStorage *storage,
                                       ThrottleCalibrationData *throttle_calibration);
