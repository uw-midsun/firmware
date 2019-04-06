#include <stdbool.h>
#include <stdio.h>

#include "steering_angle.h"
#include "steering_angle_calibration.h"

#include "ads1015.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pc_cfg.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"
#include "wait.h"

static Ads1015Storage s_ads1015;
static SteeringAngleStorage s_steering_angle_storage;
static SteeringAngleCalibrationStorage s_calibration_storage;

void setup_test(void) {
  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();

  GpioAddress ready_pin = PC_CFG_PEDAL_ADC_RDY_PIN;
  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = PC_CFG_I2C_BUS_SCL,
    .sda = PC_CFG_I2C_BUS_SDA,
  };

  SteeringAngleSettings calib_settings = {
    .ads1015 = &s_ads1015,
    .adc_channel = ADS1015_CHANNEL_3,
  };
  steering_angle_calib_init(&s_calibration_storage, &calib_settings);

  i2c_init(PC_CFG_I2C_BUS_PORT, &i2c_settings);

  ads1015_init(s_calibration_storage.settings->ads1015, PC_CFG_I2C_BUS_PORT, ADS1015_ADDRESS_GND,
               &ready_pin);
}

void teardown_test(void) {}

void test_steering_angle(void) {
  LOG_DEBUG("Please fully turn the angle in the counter-clockwise direction \n");
  delay_s(7);
  calc_boundary(&s_calibration_storage, &s_calibration_storage.data.min_reading);
  LOG_DEBUG(" %d \n", s_calibration_storage.data.min_reading);

  LOG_DEBUG("Please fully turn the angle in the clockwise direction \n");
  delay_s(7);
  calc_boundary(&s_calibration_storage, &s_calibration_storage.data.max_reading);
  LOG_DEBUG(" %d \n", s_calibration_storage.data.max_reading);

  SteeringAngleCalibrationData calib_data;
  steering_angle_calib_result(&s_calibration_storage, &calib_data);
  steering_angle_init(&s_steering_angle_storage, &calib_data, s_calibration_storage.settings);
  LOG_DEBUG("Midpoint: %d \n", calib_data.angle_midpoint);
  LOG_DEBUG("Max-bound %d \n", calib_data.max_bound);
  LOG_DEBUG("Min-bound: %d \n", calib_data.min_bound);
}
