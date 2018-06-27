#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rotary_sensor.h"

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
  RotarySensorStorage *storage = context;
  uint16_t sensor_reading = INT16_MAX;

  StatusCode read_status = adc_read_raw(storage->calibration_data->sensor_channel, &sensor_reading);

  if (status_ok(read_status)) {
    storage->sensor_steering_percent =
        prv_get_wheel_percentage(sensor_reading, storage->calibration_data->rotary_midpoint,
                                 storage->calibration_data->rotary_midpoint);
  }
}

StatusCode rotary_sensor_init(RotarySensorStorage *storage,
                              RotarySensorCalibrationData *calibration_data) {
  if (storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;

  return soft_timer_start_millis(ROTARY_SENSOR_UPDATE_PERIOD_MS, prv_raise_event_timer_callback,
                                 storage, NULL);
}

// RotarySensorCalibrationData *rotary_sensor_calib_data =
// s_rotary_sensor_storage.calibration_data;
