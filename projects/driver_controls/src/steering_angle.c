#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "steering_angle.h"

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Converts digital reading to percentage
static StatusCode prv_get_angle_percentage(SteeringAngleStorage *steering_angle_storage,
                                           uint16_t reading) {
  int16_t percentage =
      (int16_t)((reading - steering_angle_storage->calibration_data->angle_midpoint) * 100 /
                steering_angle_storage->calibration_data->angle_midpoint);

  if (percentage > 100 || percentage < -100) {
    return STATUS_CODE_OUT_OF_RANGE;
  }
  steering_angle_storage->angle_steering_percent = percentage;
  return STATUS_CODE_OK;
}

// converts analog to digital and passes the digital conversion to our percentage
// conversion function
StatusCode steering_angle_get_position(SteeringAngleStorage *steering_angle_storage) {
  if (steering_angle_storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  uint16_t reading = 0;
  StatusCode read_status =
      adc_read_raw(steering_angle_storage->calibration_data->angle_channel, &reading);
  prv_get_angle_percentage(steering_angle_storage, reading);
  if (status_ok(read_status)) {
    return prv_get_angle_percentage(steering_angle_storage, reading);
  } else {
    return read_status;
  }
}
// Initializes steering angle storage wtih calibration data
StatusCode steering_angle_init(SteeringAngleStorage *storage,
                               SteeringAngleCalibrationData *calibration_data) {
  if (storage == NULL || calibration_data == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;
  return STATUS_CODE_OK;
}

// SteeringAngleCalibrationData *steering_angle_calib_data =
// s_steering_angle_storage.calibration_data;
