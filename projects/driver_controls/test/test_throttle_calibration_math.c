#include "unity.h"
#include "test_helpers.h"
#include "throttle_calibration.h"
#include <string.h>
#include "log.h"

static ThrottleCalibrationStorage s_calib_storage;

void setup_test(void) {
}

void teardown_test(void) {
}

void test_throttle_calibration_math(void) {
  ThrottleCalibrationSettings calib_settings = {
    .adc_channel = { ADS1015_CHANNEL_0, ADS1015_CHANNEL_1 },
    .zone_percentage = { 40, 10, 50 },
    .sync_safety_factor = 2,
    .bounds_tolerance_percentage = 5
  };
  throttle_calibration_init(&s_calib_storage, &calib_settings);

  ThrottleCalibrationPointData data[NUM_THROTTLE_CALIBRATION_CHANNELS][NUM_THROTTLE_CALIBRATION_POINTS] = {
    [THROTTLE_CALIBRATION_CHANNEL_A] = {
      [THROTTLE_CALIBRATION_POINT_FULL_BRAKE] = {
        .min_reading = 100,
        .max_reading = 100,
      },
      [THROTTLE_CALIBRATION_POINT_FULL_ACCEL] = {
        .min_reading = 300,
        .max_reading = 300,
      },
    },
    [THROTTLE_CALIBRATION_CHANNEL_B] = {
      [THROTTLE_CALIBRATION_POINT_FULL_BRAKE] = {
        .min_reading = 600,
        .max_reading = 630,
      },
      [THROTTLE_CALIBRATION_POINT_FULL_ACCEL] = {
        .min_reading = 2000,
        .max_reading = 2100,
      },
    },
  };

  memcpy(s_calib_storage.data, data, sizeof(s_calib_storage.data));

  ThrottleCalibrationData calib_data = { 0 };
  throttle_calibration_result(&s_calib_storage, &calib_data);

  const char *zones[NUM_THROTTLE_ZONES] = { "Brake", "Coast", "Accel" };
  int16_t prev_max = INT16_MIN;
  for (ThrottleZone zone = 0; zone < NUM_THROTTLE_ZONES; zone++) {
    ThrottleZoneThreshold *threshold = &calib_data.zone_thresholds_main[zone];
    LOG_DEBUG("%s: min %d max %d\n", zones[zone], threshold->min, threshold->max);
    TEST_ASSERT(prev_max < threshold->min);
    TEST_ASSERT(threshold->min < threshold->max);
    prev_max = threshold->max;
  }
  for (ThrottleChannel channel = 0; channel < NUM_THROTTLE_CHANNELS; channel++) {
    ThrottleLine *line = &calib_data.line[channel];
    LOG_DEBUG("Channel %d: %d @ brake, %d @ accel\n", channel, line->full_brake_reading, line->full_accel_reading);
  }
  LOG_DEBUG("Tolerance: %d\n", calib_data.tolerance);
}
