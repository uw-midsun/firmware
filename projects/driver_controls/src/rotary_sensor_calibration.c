#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rotary_sensor.h"
#include "rotary_sensor_calibration.h"

#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

StatusCode rotary_sensor_calib_init(RotarySensorCalibrationStorage *storage,
                                    RotarySensorCalibrationSettings *settings) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}

void prv_calc_boundary(ADCChannel read_channel, uint16_t *boundary_reading) {
  adc_read_raw(read_channel, boundary_reading);
}

static uint16_t prv_calc_range(RotarySensorCalibrationPointData channel_data) {
  return channel_data.max_reading - channel_data.min_reading;
}

static uint16_t prv_calc_midpoint(RotarySensorCalibrationPointData point_data) {
  return (point_data.max_reading + point_data.min_reading) / 2;
}

StatusCode rotary_sensor_calib_result(RotarySensorCalibrationStorage *storage,
                                      RotarySensorCalibrationData *calib_data) {
  memset(calib_data, 0, sizeof(*calib_data));

  calib_data->sensor_channel = storage->settings.adc_channel;
  calib_data->rotary_range = prv_calc_range(storage->data);
  calib_data->rotary_midpoint = prv_calc_midpoint(storage->data);

  return STATUS_CODE_OK;
}
