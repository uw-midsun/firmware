#include "pedal_calibration.h"
#include "string.h"

StatusCode pedal_calibration_init(PedalCalibrationStorage *storage, Ads1015Storage *ads1015_storage,
                                  Ads1015Channel channel_a, Ads1015Channel channel_b) {
  memset(storage, 0, sizeof(*storage));
  storage->ads1015_storage = ads1015_storage;
  storage->channel_a = channel_a;
  storage->channel_b = channel_b;
  status_ok_or_return(ads1015_configure_channel(ads1015_storage, channel_a, true, NULL, NULL));
  status_ok_or_return(ads1015_configure_channel(ads1015_storage, channel_b, true, NULL, NULL));
  return STATUS_CODE_OK;
}

StatusCode pedal_calibration_get_band(PedalCalibrationStorage *storage,
                                      PedalCalibrationState state) {
  int16_t *min_a = &storage->band[PEDAL_CALIBRATION_CHANNEL_A][state].min;

  int16_t *max_a = &storage->band[PEDAL_CALIBRATION_CHANNEL_A][state].max;

  int16_t *min_b = &storage->band[PEDAL_CALIBRATION_CHANNEL_B][state].min;

  int16_t *max_b = &storage->band[PEDAL_CALIBRATION_CHANNEL_B][state].max;

  int16_t reading_a;
  int16_t reading_b;

  status_ok_or_return(ads1015_read_raw(storage->ads1015_storage, storage->channel_a, &reading_a));
  *min_a = reading_a;
  *max_a = reading_a;

  status_ok_or_return(ads1015_read_raw(storage->ads1015_storage, storage->channel_b, &reading_b));
  *min_b = reading_b;
  *max_b = reading_b;

  for (uint16_t i = 0; i < PEDAL_CALIBRATION_NUM_SAMPLES; i++) {
    status_ok_or_return(ads1015_read_raw(storage->ads1015_storage, storage->channel_a, &reading_a));

    if (reading_a < *min_a) {
      *min_a = reading_a;
    } else if (reading_a > *max_a) {
      *max_a = reading_a;
    }

    status_ok_or_return(ads1015_read_raw(storage->ads1015_storage, storage->channel_b, &reading_b));

    if (reading_b < *min_b) {
      *min_b = reading_b;
    } else if (reading_b > *max_b) {
      *max_b = reading_b;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode pedal_calibration_calc(PedalCalibrationStorage *storage,
                                  ThrottleCalibrationData *throttle_calibration,
                                  uint8_t brake_zone_percentage, uint8_t coast_zone_percentage) {
  int16_t range_a =
      storage->band[PEDAL_CALIBRATION_CHANNEL_A][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
      storage->band[PEDAL_CALIBRATION_CHANNEL_A][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
  int16_t range_b =
      storage->band[PEDAL_CALIBRATION_CHANNEL_B][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
      storage->band[PEDAL_CALIBRATION_CHANNEL_B][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
  PedalCalibrationChannel main_channel = PEDAL_CALIBRATION_CHANNEL_A;
  PedalCalibrationChannel secondary_channel = PEDAL_CALIBRATION_CHANNEL_B;
  if (range_a < range_b) {
    main_channel = PEDAL_CALIBRATION_CHANNEL_B;
    secondary_channel = PEDAL_CALIBRATION_CHANNEL_A;
    Ads1015Channel temp = storage->channel_a;
    storage->channel_a = storage->channel_b;
    storage->channel_b = temp;
  }

  int16_t min_brake = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  int16_t max_brake = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].max;

  int16_t min_accel = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].min;

  int16_t max_accel = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max;

  throttle_calibration->line[THROTTLE_CHANNEL_MAIN].full_brake_reading =
      (min_brake + max_brake) / 2;
  throttle_calibration->line[THROTTLE_CHANNEL_MAIN].full_throttle_reading =
      (min_accel + max_accel) / 2;
  int16_t range = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
                  storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  ThrottleZoneThreshold *zone_thresholds = throttle_calibration->zone_thresholds_main;

  zone_thresholds[THROTTLE_ZONE_BRAKE].min = min_brake;
  zone_thresholds[THROTTLE_ZONE_BRAKE].max =
      zone_thresholds[THROTTLE_ZONE_BRAKE].min + (brake_zone_percentage * range / 100);

  zone_thresholds[THROTTLE_ZONE_COAST].min = zone_thresholds[THROTTLE_ZONE_BRAKE].max + 1;
  zone_thresholds[THROTTLE_ZONE_COAST].max =
      zone_thresholds[THROTTLE_ZONE_COAST].min + (coast_zone_percentage * range / 100);

  zone_thresholds[THROTTLE_ZONE_ACCEL].min = zone_thresholds[THROTTLE_ZONE_COAST].max + 1;
  zone_thresholds[THROTTLE_ZONE_ACCEL].max = max_accel;

  min_brake = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  max_brake = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].max;

  min_accel = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].min;

  max_accel = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max;

  throttle_calibration->line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading =
      (min_brake + max_brake) / 2;
  throttle_calibration->line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading =
      (min_accel + max_accel) / 2;
  throttle_calibration->tolerance = MAX((max_brake - min_brake) / 2, (max_accel - min_accel) / 2);

  return STATUS_CODE_OK;
}
