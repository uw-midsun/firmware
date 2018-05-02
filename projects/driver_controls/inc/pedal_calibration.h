#pragma once
// This module provides the means for pedal calibration.
// Throttle module takes in a structure, ThrottleCalibrationData, that is calibrated in this module.
//
// Calibration has basically 3 stages. First stage is about reading and recording pedal inputs from
// pedal at full brake position. The second stage is same as first but for full throttle position.
// The last stage is about calculating the results into a ThrottleCalibrationData structure that
// throttle module uses.
//
// The procedure is set up in test_pedal_calibration with the help of a FSM that
// when switched to a state calls the corresponding functions from this module. Look at
// pedal_calibration_fsm module for more information.
#include "ads1015.h"
#include "throttle.h"

// Pedal takes 2 ADC channels.
// Calibration determines the high and the low resolution channel.
typedef enum {
  PEDAL_CALIBRATION_CHANNEL_A = 0,
  PEDAL_CALIBRATION_CHANNEL_B,
  NUM_PEDAL_CALIBRATION_CHANNELS
} PedalCalibrationChannel;

// Pedal inputs are read in 2 positions: full throttle and full brake.
typedef enum {
  PEDAL_CALIBRATION_STATE_FULL_BRAKE = 0,  // Fully pressed.
  PEDAL_CALIBRATION_STATE_FULL_THROTTLE,   // Fully depressed.
  NUM_PEDAL_CALIBRATION_STATES
} PedalCalibrationState;

// The range is the structure that holds max and min ADC readings on a channel in a fixed state.
// For example, reading the pedal at full brake position would give a range of values, not one.
typedef struct PedalCalibrationRange {
  int16_t min;
  int16_t max;
} PedalCalibrationRange;

typedef struct PedalCalibrationSettings {
  Ads1015Storage *ads1015_storage;
  // This should be the final product of calibration ready to be used by throttle module.
  ThrottleCalibrationData *throttle_calibration_data;
  Ads1015Channel adc_channel[NUM_PEDAL_CALIBRATION_CHANNELS];
  uint8_t brake_zone_percentage; // [0, x]
  uint8_t coast_zone_percentage; // (x, y]
  uint8_t sync_safety_factor; // difference between primary/secondary channels - multiplier
  uint8_t bounds_tolerance; // min/max bounds tolerance - percentage
} PedalCalibrationSettings;

typedef struct PedalCalibrationStorage {
  // TODO: remove from this
  ThrottleStorage throttle;
  PedalCalibrationSettings settings;
  // Obtained ranges from reading the pedal in the two states. On each channel, connecting the
  // points form a band of data (on the position-voltage graph).
  PedalCalibrationRange band[NUM_PEDAL_CALIBRATION_CHANNELS][NUM_PEDAL_CALIBRATION_STATES];
  PedalCalibrationState state;
  volatile uint32_t sample_counter[NUM_PEDAL_CALIBRATION_CHANNELS];
} PedalCalibrationStorage;

// Initializes the pedal calibration storage. Ads1015Storage should be initialized.
// The zone percentages divide the pedal's range of motion into three zones (brake, coast, accel).
// Tolerance safety factor is to account for conditions not present during calibration.
StatusCode pedal_calibration_init(PedalCalibrationStorage *storage, PedalCalibrationSettings *settings);

// Finds and stores max and min readings from pedal in a given state for both channels.
// The pedal should be kept at the given state until the function finishes sampling.
// It samples a known number of times.
StatusCode pedal_calibration_process_state(PedalCalibrationStorage *storage,
                                           PedalCalibrationState state);

// This should be called only after calling pedal_calibration_process_state for both states.
// It initializes ThrottleCalibrationData based on the read data in PedalCalibrationStorage.
// This structure could then be passed to throttle module assuming calibration was successful.
StatusCode pedal_calibration_calculate(PedalCalibrationStorage *storage);
