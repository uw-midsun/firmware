#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>
#include "ads1015.h"
#include "delay.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mech_brake_calibration.h"
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

  // enables channel
  ads1015_configure_channel(storage->settings.ads1015, storage->settings.channel, true,
                            prv_callback_channel, storage);

  bool samples_completed = false;
  do {
    wait();
    samples_completed = true;
    samples_completed &=
        (storage->data[point].sample_counter >= MECH_BRAKE_CALIBRATION_NUM_SAMPLES);
  } while (!samples_completed);

  return STATUS_CODE_OK;
}

StatusCode mech_brake_get_calib_data(MechBrakeCalibrationStorage *storage, MechBrakeCalibrationData *calib_data){
  
  if (storage == NULL || calib_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  int16_t max_unpressed_reading = storage->data[MECH_BRAKE_CALIBRATION_POINT_UNPRESSED].max_reading;
  int16_t min_unpressed_reading = storage->data[MECH_BRAKE_CALIBRATION_POINT_UNPRESSED].min_reading;
  int16_t max_pressed_reading = storage->data[MECH_BRAKE_CALIBRATION_POINT_PRESSED].max_reading;
  int16_t min_pressed_reading = storage->data[MECH_BRAKE_CALIBRATION_POINT_PRESSED].min_reading;

  int16_t average_unpressed_value = (max_unpressed_reading + min_unpressed_reading) / 2;
  int16_t average_pressed_value = (max_pressed_reading + min_pressed_reading) / 2;
  
  calib_data->zero_value = average_unpressed_value;
  calib_data->hundred_value = average_pressed_value;

  return STATUS_CODE_OK;
}



