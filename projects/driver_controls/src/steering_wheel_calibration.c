#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "steering_wheel.h"
#include "steering_wheel_calibration.h"

#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

StatusCode steering_wheel_calib_init(SteeringWheelCalibrationStorage *storage,
                                     SteeringWheelCalibrationSettings *settings) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}

void prv_calc_boundary(ADCChannel read_channel, uint16_t *boundary_reading) {
  adc_read_raw(read_channel, boundary_reading);
}

static uint16_t prv_calc_range(SteeringWheelCalibrationPointData channel_data) {
  return channel_data.max_reading - channel_data.min_reading;
}

static uint16_t prv_calc_midpoint(SteeringWheelCalibrationPointData point_data) {
  return (point_data.max_reading + point_data.min_reading) / 2;
}

StatusCode steering_wheel_calib_result(SteeringWheelCalibrationStorage *storage,
                                       SteeringWheelCalibrationData *calib_data) {
  memset(calib_data, 0, sizeof(*calib_data));

  calib_data->wheel_channel = storage->settings.adc_channel;
  calib_data->wheel_range = prv_calc_range(storage->data);
  calib_data->wheel_midpoint = prv_calc_midpoint(storage->data);
  calib_data->max_bound = storage->data.max_reading;
  calib_data->min_bound = storage->data.min_reading;
  return STATUS_CODE_OK;
}
