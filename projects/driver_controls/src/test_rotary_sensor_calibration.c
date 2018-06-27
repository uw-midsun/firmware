#include <stdbool.h>
#include <stdio.h>

#include "rotary_sensor.h"
#include "rotary_sensor_calibration.h"

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static RotarySensorStorage s_rotary_sensor_storage;
static RotarySensorCalibrationStorage s_calibration_storage;

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

  RotarySensorCalibrationSettings calib_settings = {
    .adc_channel = conversion_channel,
  };

  rotary_sensor_calib_init(&s_calibration_storage, &calib_settings);
}

void teardown_test(void) {}

void test_rotary_sensor(void) {
  LOG_DEBUG("Please fully turn the wheel in the counter-clockwise direction");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel, &s_calibration_storage.data.min_reading);

  LOG_DEBUG("Please fully turn the wheel in the clockwise direction");
  delay_s(7);
  prv_calc_boundary(s_calibration_storage.settings.adc_channel, &s_calibration_storage.data.max_reading);

  RotarySensorCalibrationData calib_data;
  rotary_sensor_calib_result(&s_calibration_storage, &calib_data);
  rotary_sensor_init(&s_rotary_sensor_storage, &calib_data);
}
