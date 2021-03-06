#include "mech_brake_calibration.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ads1015.h"
#include "delay.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"
#include "wait.h"

static void prv_callback_channel(Ads1015Channel ads1015_channel, void *context) {
  MechBrakeCalibrationStorage *storage = context;

  MechBrakeCalibrationPointData *data = &storage->data[storage->sample_point];
  if (data->sample_counter >= MECH_BRAKE_CALIBRATION_NUM_SAMPLES) {
    ads1015_configure_channel(storage->settings.ads1015, ads1015_channel, false, NULL, NULL);
  } else {
    int16_t reading = 0;
    ads1015_read_raw(storage->settings.ads1015, ads1015_channel, &reading);

    data->sample_counter++;
    data->min_reading = MIN(data->min_reading, reading);
    data->max_reading = MAX(data->max_reading, reading);
  }
}

StatusCode mech_brake_calibration_init(MechBrakeCalibrationStorage *storage,
                                       MechBrakeSettings *settings) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}

StatusCode mech_brake_sample(MechBrakeCalibrationStorage *storage,
                             MechBrakeCalibrationPoint point) {
  // Disables channel
  ads1015_configure_channel(storage->settings.ads1015, storage->settings.channel, false, NULL,
                            NULL);

  storage->data[point] = (MechBrakeCalibrationPointData){
    .sample_counter = 0,
    .min_reading = INT16_MAX,
    .max_reading = INT16_MIN,
  };
  storage->sample_point = point;

  // Enables channel
  ads1015_configure_channel(storage->settings.ads1015, storage->settings.channel, true,
                            prv_callback_channel, storage);

  while (storage->data[point].sample_counter < MECH_BRAKE_CALIBRATION_NUM_SAMPLES) {
    wait();
  }

  int16_t max_reading = storage->data[point].max_reading;
  int16_t min_reading = storage->data[point].min_reading;

  storage->data[point].average_value = (max_reading + min_reading) / 2;

  return STATUS_CODE_OK;
}

StatusCode mech_brake_get_calib_data(MechBrakeCalibrationStorage *storage,
                                     MechBrakeCalibrationData *calib_data) {
  if (storage == NULL || calib_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  calib_data->zero_value = storage->data[MECH_BRAKE_CALIBRATION_POINT_UNPRESSED].average_value;
  calib_data->hundred_value = storage->data[MECH_BRAKE_CALIBRATION_POINT_PRESSED].average_value;

  return STATUS_CODE_OK;
}
