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

// Converts digital reading to percentage
static StatusCode prv_get_wheel_percentage(SteeringWheelStorage *steering_wheel_storage,
                                           uint16_t reading) {
  int16_t percentage =
      (int16_t)((reading - steering_wheel_storage->calibration_data->wheel_midpoint) * 100 /
                steering_wheel_storage->calibration_data->wheel_midpoint);

  if (percentage > 100 || percentage < -100) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  steering_wheel_storage->wheel_steering_percent = percentage;
  return STATUS_CODE_OK;
}

// converts analog to digital and passes the digital conversion to our percentage
// conversion function
StatusCode steering_wheel_get_position(SteeringWheelStorage *steering_wheel_storage) {
  if (steering_wheel_storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  uint16_t reading = 0;
  StatusCode read_status =
      adc_read_raw(steering_wheel_storage->calibration_data->wheel_channel, &reading);
  prv_get_wheel_percentage(steering_wheel_storage, reading);
  if (status_ok(read_status)) {
    return prv_get_wheel_percentage(steering_wheel_storage, reading);
  } else {
    return read_status;
  }
}
// Initializes steering wheel storage wtih calibration data
StatusCode steering_wheel_init(SteeringWheelStorage *storage,
                               SteeringWheelCalibrationData *calibration_data) {
  if (storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;
  return STATUS_CODE_OK;
}

// SteeringWheelCalibrationData *steering_wheel_calib_data =
// s_steering_wheel_storage.calibration_data;
