#pragma once
//
#include "ads1015.h"
#include "throttle.h"

#define PEDAL_CALIBRATION_NUM_SAMPLES 100

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

StatusCode pedal_calibration_init(PedalCalibrationStorage *storage, Ads1015Storage *ads1015_storage,
                                  Ads1015Channel channel_a, Ads1015Channel channel_b,
                                  uint8_t brake_zone_percentage, uint8_t coast_zone_percentage,
                                  uint8_t tolerance_safety_factor);

StatusCode pedal_calibration_process_state(PedalCalibrationStorage *storage,
                                      PedalCalibrationState state);

StatusCode pedal_calibration_calculate(PedalCalibrationStorage *storage,
                                      ThrottleCalibrationData *throttle_calibration);
