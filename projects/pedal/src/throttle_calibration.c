#include "throttle_calibration.h"
#include <string.h>
#include "log.h"
#include "wait.h"

static void prv_adc_callback(ADCChannel adc_channel, void *context) {
  ThrottleCalibrationStorage *storage = context;

  // Associate current callback's channel with the calibration channel
  ThrottleCalibrationChannel calib_channel = NUM_THROTTLE_CALIBRATION_CHANNELS;
  for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
    if (storage->settings.adc_channel[i] == adc_channel) {
      calib_channel = i;
      break;
    }
  }

  ThrottleCalibrationPointData *data = &storage->data[calib_channel][storage->sample_point];
  if (data->sample_counter >= THROTTLE_CALIBRATION_NUM_SAMPLES) {
    adc_set_channel(adc_channel, false);
  } else {
    uint16_t reading = 0;
    adc_read_raw(adc_channel, &reading);

    data->sample_counter++;
    data->min_reading = MIN(data->min_reading, reading);
    data->max_reading = MAX(data->max_reading, reading);
  }
}

StatusCode throttle_calibration_init(ThrottleCalibrationStorage *storage,
                                     ThrottleCalibrationSettings *settings) {
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
StatusCode throttle_calibration_sample(ThrottleCalibrationStorage *storage,
                                       ThrottleCalibrationPoint point) {
  // Disable channels
  for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
    adc_set_channel(storage->settings.adc_channel[i], false);
  }

  for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
    storage->data[i][point] = (ThrottleCalibrationPointData){
      .sample_counter = 0,
      .min_reading = INT16_MAX,
      .max_reading = INT16_MIN,
    };
  }
  storage->sample_point = point;

  // Enable channels with new point data context
  for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
    adc_register_callback(storage->settings.adc_channel[i], prv_adc_callback, storage);
    adc_set_channel(storage->settings.adc_channel[i], true);
  }

  bool samples_completed = false;
  do {
    wait();
    samples_completed = true;
    for (size_t i = 0; i < NUM_THROTTLE_CALIBRATION_CHANNELS; i++) {
      samples_completed &=
          (storage->data[i][point].sample_counter >= THROTTLE_CALIBRATION_NUM_SAMPLES);
    }
  } while (!samples_completed);

  return STATUS_CODE_OK;
}

static int16_t prv_calc_range(ThrottleCalibrationPointData *channel_data) {
  return channel_data[THROTTLE_CALIBRATION_POINT_FULL_ACCEL].max_reading -
         channel_data[THROTTLE_CALIBRATION_POINT_FULL_BRAKE].min_reading;
}

static int16_t prv_calc_midpoint(ThrottleCalibrationPointData *point_data) {
  return (point_data->max_reading + point_data->min_reading) / 2;
}

static void prv_calc_line(ThrottleLine *line, ThrottleCalibrationPointData *channel_data) {
  line->full_brake_reading =
      prv_calc_midpoint(&channel_data[THROTTLE_CALIBRATION_POINT_FULL_BRAKE]);
  line->full_accel_reading =
      prv_calc_midpoint(&channel_data[THROTTLE_CALIBRATION_POINT_FULL_ACCEL]);
}

// Calculates and stores calibration settings
StatusCode throttle_calibration_result(ThrottleCalibrationStorage *storage,
                                       ThrottleCalibrationData *calib_data) {
  memset(calib_data, 0, sizeof(*calib_data));

  int16_t range_a = prv_calc_range(storage->data[THROTTLE_CALIBRATION_CHANNEL_A]),
          range_b = prv_calc_range(storage->data[THROTTLE_CALIBRATION_CHANNEL_B]);
  ThrottleCalibrationChannel main_channel = THROTTLE_CALIBRATION_CHANNEL_A,
                             secondary_channel = THROTTLE_CALIBRATION_CHANNEL_B;

  if (range_a < range_b) {
    main_channel = THROTTLE_CALIBRATION_CHANNEL_B;
    secondary_channel = THROTTLE_CALIBRATION_CHANNEL_A;
  }

  prv_calc_line(&calib_data->line[THROTTLE_CHANNEL_MAIN], storage->data[main_channel]);
  prv_calc_line(&calib_data->line[THROTTLE_CHANNEL_SECONDARY], storage->data[secondary_channel]);

  calib_data->channel_main = storage->settings.adc_channel[main_channel];
  calib_data->channel_secondary = storage->settings.adc_channel[secondary_channel];

  // Get the full range for main channel so we can calculate the size of each zone.
  int16_t range = prv_calc_range(storage->data[main_channel]);
  int16_t range_tolerance = range * storage->settings.bounds_tolerance_percentage / 100;
  range += range_tolerance;
  // Increase range? - bounds tolerance

  // Use the percentages to calculate the thresholds of each zone.
  ThrottleZoneThreshold *brake_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_BRAKE],
                        *coast_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_COAST],
                        *accel_threshold = &calib_data->zone_thresholds_main[THROTTLE_ZONE_ACCEL];

  ThrottleCalibrationPointData *main_data = storage->data[main_channel];
  brake_threshold->min =
      main_data[THROTTLE_CALIBRATION_POINT_FULL_BRAKE].min_reading - range_tolerance;
  brake_threshold->max =
      brake_threshold->min + (storage->settings.zone_percentage[THROTTLE_ZONE_BRAKE] * range / 100);

  coast_threshold->min = brake_threshold->max + 1;
  coast_threshold->max =
      coast_threshold->min + (storage->settings.zone_percentage[THROTTLE_ZONE_COAST] * range / 100);

  accel_threshold->min = coast_threshold->max + 1;
  accel_threshold->max =
      main_data[THROTTLE_CALIBRATION_POINT_FULL_ACCEL].max_reading + range_tolerance;

  // TODO(ELEC-414): Need to figure out tolerance calculation
  calib_data->tolerance = 20;

  printf("brake: %d - %d\n", brake_threshold->min, brake_threshold->max);
  printf("coast: %d - %d\n", coast_threshold->min, coast_threshold->max);
  printf("accel: %d - %d\n", accel_threshold->min, accel_threshold->max);

  return STATUS_CODE_OK;
}
