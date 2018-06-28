#include <stdbool.h>
#include <stdio.h>

#include "steering_wheel.h"
#include "steering_wheel_calibration.h"

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static SteeringWheelStorage s_steering_wheel_storage;
static SteeringWheelCalibrationStorage s_calibration_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  ADCChannel conversion_channel;
  const GPIOAddress conversion_address = {
    .port = GPIO_PORT_A,
    .pin = 7,
  };

  adc_init(ADC_MODE_CONTINUOUS);
  adc_get_channel(conversion_address, &conversion_channel);
  adc_set_channel(conversion_channel, true);

  SteeringWheelCalibrationSettings calib_settings = {
    .adc_channel = conversion_channel,
  };

  steering_wheel_calib_init(&s_calibration_storage, &calib_settings);
}

void teardown_test(void) {}

void test_steering_wheel(void) {
  LOG_DEBUG("Please fully turn the wheel in the counter-clockwise direction");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel,
                    &s_calibration_storage.data.min_reading);

  LOG_DEBUG("Please fully turn the wheel in the clockwise direction");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel,
                    &s_calibration_storage.data.max_reading);

  SteeringWheelCalibrationData calib_data;
  steering_wheel_calib_result(&s_calibration_storage, &calib_data);
  steering_wheel_init(&s_steering_wheel_storage, &calib_data);
}
