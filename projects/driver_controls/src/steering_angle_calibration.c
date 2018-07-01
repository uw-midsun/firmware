#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "steering_angle.h"
#include "steering_angle_calibration.h"

#include "ads1015.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Initializes calibration storage with predefined settings
StatusCode steering_angle_calib_init(SteeringAngleCalibrationStorage *storage,
                                     SteeringAngleCalibrationSettings *settings) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}
// calculations for range, boundary, and midpoint
void prv_calc_boundary(SteeringAngleCalibrationStorage *storage, int16_t *boundary_reading) {
  ads1015_configure_channel(storage->settings.ads1015, storage->settings.adc_channel, true, NULL,
                            NULL);
  ads1015_read_raw(storage->settings.ads1015, storage->settings.adc_channel, boundary_reading);
}

static uint16_t prv_calc_midpoint(SteeringAngleCalibrationPointData point_data) {
  return (point_data.max_reading + point_data.min_reading) / 2;
}
// primary calibration function, calibrates the steering_angle calibration data
// From calibration storage
StatusCode steering_angle_calib_result(SteeringAngleCalibrationStorage *storage,
                                       SteeringAngleCalibrationData *calib_data) {
  memset(calib_data, 0, sizeof(*calib_data));

  // calib_data->angle_channel = storage->settings.adc_channel;
  calib_data->angle_midpoint = prv_calc_midpoint(storage->data);
  calib_data->max_bound = storage->data.max_reading;
  calib_data->min_bound = storage->data.min_reading;
  calib_data->tolerance_percentage = 3;
  return STATUS_CODE_OK;
}
