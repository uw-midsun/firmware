#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "steering_wheel.h"

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static uint16_t prv_get_wheel_percentage(uint16_t reading, uint16_t range, uint16_t midpoint) {
  return (reading * 100) / range;
}

static void prv_raise_event_timer_callback(SoftTimerID timer_id, void *context) {
  SteeringWheelStorage *storage = context;
  uint16_t sensor_reading = INT16_MAX;

  StatusCode read_status = adc_read_raw(storage->calibration_data->sensor_channel, &sensor_reading);

  if (status_ok(read_status)) {
    storage->sensor_steering_percent =
        prv_get_wheel_percentage(sensor_reading, storage->calibration_data->rotary_midpoint,
                                 storage->calibration_data->rotary_midpoint);
  }
}

StatusCode steering_wheel_init(SteeringWheelStorage *storage,
                              SteeringWheelCalibrationData *calibration_data) {
  if (storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;

  return soft_timer_start_millis(steering_wheel_UPDATE_PERIOD_MS, prv_raise_event_timer_callback,
                                 storage, NULL);
}

// SteeringWheelCalibrationData *steering_wheel_calib_data =
// s_steering_wheel_storage.calibration_data;
