#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "steering_angle.h"

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
#include "dc_cfg.h"

// Converts digital reading to percentage
static StatusCode prv_get_angle_percentage(SteeringAngleStorage *steering_angle_storage,
                                           int16_t reading) {
  int16_t percentage =
      (int16_t)((reading - steering_angle_storage->calibration_data->angle_midpoint) * 100 /
                steering_angle_storage->calibration_data->angle_midpoint);

  if (percentage > (100 + steering_angle_storage->calibration_data->tolerance_percentage) ||
      percentage < (-100 - steering_angle_storage->calibration_data->tolerance_percentage)) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
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
  int16_t reading = 0;

  StatusCode read_status = ads1015_read_raw(steering_angle_storage->ads1015,
                                            steering_angle_storage->adc_channel, &reading);

  prv_get_angle_percentage(steering_angle_storage, reading);
  if (status_ok(read_status)) {
    return prv_get_angle_percentage(steering_angle_storage, reading);
  } else {
    return status_code(read_status);
  }
}

// test version of the steering_angle_get_position function (allows to set any desired reading
// independent of the adc)
StatusCode steering_angle_get_position_test(SteeringAngleStorage *steering_angle_storage,
                                            int16_t reading) {
  if (steering_angle_storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return prv_get_angle_percentage(steering_angle_storage, reading);
}

// Initializes steering angle storage wtih calibration data
StatusCode steering_angle_init(SteeringAngleStorage *storage,
                               SteeringAngleCalibrationData *calibration_data,
                               SteeringAngleSettings *settings) {
  if (storage == NULL || calibration_data == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;
  storage->adc_channel = settings->adc_channel;
  storage->ads1015 = settings->ads1015;

  ads1015_configure_channel(storage->ads1015, storage->adc_channel, true, NULL,
                            NULL);

  delay_ms(10);
  return STATUS_CODE_OK;
}

// SteeringAngleCalibrationData *steering_angle_calib_data =
// s_steering_angle_storage.calibration_dstatuscode(ata;
