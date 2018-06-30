#include <stdbool.h>
#include <stdio.h>

#include "steering_angle.h"
#include "steering_angle_calibration.h"

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static SteeringAngleStorage s_steering_angle_storage;
static SteeringAngleCalibrationStorage s_calibration_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  ADCChannel conversion_channel;
  const GPIOAddress conversion_address = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };

  adc_init(ADC_MODE_CONTINUOUS);
  adc_get_channel(conversion_address, &conversion_channel);
  adc_set_channel(conversion_channel, true);

  SteeringAngleCalibrationSettings calib_settings = {
    .adc_channel = conversion_channel,
  };

  steering_angle_calib_init(&s_calibration_storage, &calib_settings);
}

void teardown_test(void) {}

void test_steering_angle(void) {
  LOG_DEBUG("Please fully turn the angle in the counter-clockwise direction \n");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel,
                    &s_calibration_storage.data.min_reading);
  LOG_DEBUG(" %d \n", s_calibration_storage.data.min_reading);

  LOG_DEBUG("Please fully turn the angle in the clockwise direction \n");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel,
                    &s_calibration_storage.data.max_reading);
  LOG_DEBUG(" %d \n", s_calibration_storage.data.max_reading);

  SteeringAngleCalibrationData calib_data;
  steering_angle_calib_result(&s_calibration_storage, &calib_data);
  steering_angle_init(&s_steering_angle_storage, &calib_data);
  LOG_DEBUG("Range: %d \n", calib_data.angle_range);
  LOG_DEBUG("Midpoint: %d \n", calib_data.angle_midpoint);
  LOG_DEBUG("Max-bound %d \n", calib_data.max_bound);
  LOG_DEBUG("Min-bound: %d \n", calib_data.min_bound);
}
