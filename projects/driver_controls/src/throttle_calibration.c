#include "throttle_calibration.h"
#include <string.h>
#include "wait.h"

static void prv_adc_callback(Ads1015Channel ads1015_channel, void *context) {
  ThrottleCalibrationStorage *storage = context;

  // Associate current callback's channel with the calibration channel
  ThrottleCalibrationChannel calib_channel = NUM_THROTTLE_CALIBRATION_CHANNELS;
  for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
    if (storage->adc_channel[i] == ads1015_channel) {
      calib_channel = i;
      break;
    }
  }

  ThrottleCalibrationPointData *data = &storage->data[calib_channel][storage->sample_point];
  if (data->sample_counter >= THROTTLE_CALIBRATION_NUM_SAMPLES) {
    ads1015_configure_channel(storage->settings.ads1015, ads1015_channel, false, NULL, NULL);
  } else {
    int16_t reading = 0;
    ads1015_read_raw(storage->settings.ads1015_storage, ads1015_channel, &reading);

    data->sample_counter++;
    data->min_reading = MIN(data->min_reading, reading);
    data->max_reading = MIN(data->max_reading, reading);
  }
}

StatusCode throttle_calibration_init(ThrottleCalibrationStorage *storage, ThrottleCalibrationSettings *settings) {
  uint32_t total_percentage = 0;
  for (size_t i = 0; i < NUM_THROTTLE_ZONES; i++) {
    total_percentage += settings->zone_percentage[i];
  }
  if (total_percentage != 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Zones did not add up to 100");
  }

  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}

// Samples at the specified point - blocks until completion
StatusCode throttle_calibration_sample(ThrottleCalibrationStorage *storage, ThrottleCalibrationPoint point) {
  // Disable channels
  for (size_t i = 0; i < NUM_PEDAL_CALIBRATION_CHANNELS; i++) {
    ads1015_configure_channel(storage->settings.ads1015, storage->settings.adc_channel[i], false, NULL, NULL);
  }

  for (size_t i = 0; i < NUM_PEDAL_CALIBRATION_CHANNELS; i++) {
    storage->data[i][point] = (ThrottleCalibrationPointData) {
      .sample_counter = 0,
      .min_reading = INT16_MAX,
      .max_reading = INT16_MIN,
    };
  }
  storage->sample_point = point;

  // Enable channels with new point data context
  for (size_t i = 0; i < NUM_PEDAL_CALIBRATION_CHANNELS; i++) {
    ads1015_configure_channel(storage->settings.ads1015, storage->settings.adc_channel[i], true, prv_adc_callback, storage);
  }

  bool samples_completed = false;
  while (!samples_completed) {
    // wait until all samples are completed
    wait();

    for (size_t i = 0; i < NUM_PEDAL_CALIBRATION_CHANNELS; i++) {
      samples_completed &= (storage->data[i][point].sample_counter >= THROTTLE_CALIBRATION_NUM_SAMPLES);
    }
  }

  return STATUS_CODE_OK;
}

static int16_t prv_calc_range(ThrottleCalibrationPointData *channel_data[]) {
  return channel_data[THROTTLE_CALIBRATION_POINT_FULL_ACCEL].max - channel_data[THROTTLE_CALIBRATION_POINT_FULL_BRAKE].min;
}

static int16_t prv_calc_midpoint(ThrottleCalibrationPointData *point_data) {
  return (point_data->max_reading + point_data->min_reading) / 2;
}

// Calculates and stores calibration settings
StatusCode throttle_calibration_result(ThrottleCalibrationStorage *storage, ThrottleCalibrationData *calib_data) {
  int16_t range_a = prv_calc_range(storage->data[THROTTLE_CALIBRATION_CHANNEL_A]),
          range_b = prv_calc_range(storage->data[THROTTLE_CALIBRATION_CHANNEL_B]);
  ThrottleCalibrationChannel main_channel = THROTTLE_CALIBRATION_CHANNEL_A,
                             secondary_channel = THROTTLE_CALIBRATION_CHANNEL_B;

  if (range_a < range_b) {
    main_channel = PEDAL_CALIBRATION_CHANNEL_B;
    secondary_channel = PEDAL_CALIBRATION_CHANNEL_A;
  }

  throttle_calibration->channel_main = storage->settings.adc_channel[main_channel];
  throttle_calibration->channel_secondary = storage->settings.adc_channel[secondary_channel];

  // Get the full range for main channel so we can calculate the size of each zone.
  int16_t range = prv_calc_range(storage->data[main_channel]);
  // Increase range?

  // Use the percentages to calculate the thresholds of each zone.
  ThrottleZoneThreshold *brake_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_BRAKE],
                        *coast_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_COAST],
                        *accel_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_ACCEL];

  ThrottleCalibrationPointData *main_data = storage->data[main_channel];
  brake_threshold->min = main_data[PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
  brake_threshold->max =
      brake_threshold->min + (storage->settings.zone_percentage[THROTTLE_ZONE_BRAKE] * range / 100);

  coast_threshold->min = brake_threshold->max + 1;
  coast_threshold->max =
      coast_threshold->min + (storage->settings.zone_percentage[THROTTLE_ZONE_COAST] * range / 100);

  accel_threshold->min = coast_threshold->max + 1;
  accel_threshold->max = main_data[PEDAL_CALIBRATION_STATE_FULL_ACCEL].max;

  return STATUS_CODE_OK;
}
