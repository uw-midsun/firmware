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

  GPIOAddress ready_pin = DC_CFG_STEERING_ADC_RDY_PIN;
  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = DC_CFG_I2C_BUS_SCL,
    .sda = DC_CFG_I2C_BUS_SDA,
  };

  SteeringAngleCalibrationSettings calib_settings = {
    .adc_channel = ADS1015_CHANNEL_3,
  };

  i2c_init(DC_CFG_I2C_BUS_PORT, &i2c_settings);

  ads1015_init(s_steering_angle_storage->ads1015, DC_CFG_I2C_BUS_PORT, ADS1015_ADDRESS_GND,
               &ready_pin);

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
  LOG_DEBUG("Midpoint: %d \n", calib_data.angle_midpoint);
  LOG_DEBUG("Max-bound %d \n", calib_data.max_bound);
  LOG_DEBUG("Min-bound: %d \n", calib_data.min_bound);
}
