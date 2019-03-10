#include <stdbool.h>
#include <stdio.h>

#include "steering_angle.h"
#include "steering_angle_calibration.h"

#include "ads1015.h"
#include "pc_cfg.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"
#include "wait.h"

static SteeringAngleCalibrationData s_angle_calib_data;
static SteeringAngleStorage s_steering_angle_storage;
static Ads1015Storage s_ads1015;

static int16_t s_test_reading;

static SteeringAngleSettings settings = {
  .ads1015 = &s_ads1015,
  .adc_channel = ADS1015_CHANNEL_3,
};

// preset calibration data for testing purposes
static void prv_set_calibration_data(SteeringAngleCalibrationData *calib_data) {
  calib_data->min_bound = 0;
  calib_data->max_bound = 2047;
  calib_data->angle_midpoint = 1024;
  calib_data->tolerance_percentage = 3;
}

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

  i2c_init(I2C_PORT_1, &i2c_settings);
  ads1015_init(&s_ads1015, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data, &settings);
  prv_set_calibration_data(&s_angle_calib_data);
}

void teardown_test(void) {}

void test_steering_angle_init_invalid_args(void) {
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    steering_angle_init(NULL, &s_angle_calib_data, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    steering_angle_init(&s_steering_angle_storage, NULL, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data, NULL));
}

void test_steering_angle_get_position_invalid_args(void) {
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, steering_angle_get_position(NULL));
}

void test_read_percentage(void) {
  steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data, &settings);
  steering_angle_get_position(&s_steering_angle_storage);
  LOG_DEBUG("Percent: %d \n", s_steering_angle_storage.angle_steering_percent);
}

// Tests for above_boundary (i.e. when percent > 100), test passes and can be
// accurately executed by lowering the preset test max_bound such that you can achieve a
// reading greater than the max bound
void test_angle_turn_percentage_above_bounds(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->max_bound) + 100;
  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}
// Tests for below_boundary (i.e. when percent < -100), test passes and can be
// accurately executed by increasing the preset test min_bound such that you can achieve a
// reading lower than the min bound
void test_angle_turn_percentage_below_bounds(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->min_bound) - 100;
  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}
// Test for checking if extreme boundaries are considered valid
void test_angle_turn_percentage_within_min_bound(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->min_bound) + 1;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}
void test_angle_turn_percentage_within_max_bounds(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->max_bound) - 1;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}

// Test for checking if tolerance is accounted for appropriately
void test_angle_turn_percentage_below_bounds_tolerance(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->min_bound) - 20;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}

void test_angle_turn_percentage_above_bounds_tolerance(void) {
  s_test_reading = (s_steering_angle_storage.calibration_data->max_bound) + 20;
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    steering_angle_get_position_test(&s_steering_angle_storage, s_test_reading));
}
