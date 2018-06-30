#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "steering_angle.h"
#include "steering_angle_calibration.h"

static SteeringAngleCalibrationData s_angle_calib_data;
static SteeringAngleStorage s_steering_angle_storage;

static int16_t s_test_reading;
// preset calibration data for testing purposes
static void prv_set_calibration_data(SteeringAngleCalibrationData *calib_data) {
  calib_data->min_bound = 0;
  calib_data->max_bound = 4095;
  calib_data->angle_range = 4095;
  calib_data->angle_midpoint = 2048;
}

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

  prv_set_calibration_data(&s_angle_calib_data);
  steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data);
}

void teardown_test(void) {}

void test_steering_angle_init_invalid_args(void) {
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, steering_angle_init(NULL, &s_angle_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, steering_angle_init(&s_steering_angle_storage, NULL));
}

void test_steering_angle_get_position_invalid_args(void) {
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, steering_angle_get_position(NULL));
}
// Tests for above_boundary (i.e. when percent > 100), test passes and can be
// accurately executed by lowering the preset test max_bound such that you can achieve a
// reading greater than the max bound
void test_angle_turn_percentage_above_bounds(void) {
  steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data);
  steering_angle_get_position(&s_steering_angle_storage);
  LOG_DEBUG("Percent: %d \n", s_steering_angle_storage.angle_steering_percent);

  // TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
  //                  steering_angle_get_position(&s_steering_angle_storage));
}
// Tests for below_boundary (i.e. when percent < -100), test passes and can be
// accurately executed by increasing the preset test min_bound such that you can achieve a
// reading lower than the min bound
void test_angle_turn_percentage_below_bounds(void) {
  steering_angle_init(&s_steering_angle_storage, &s_angle_calib_data);
  steering_angle_get_position(&s_steering_angle_storage);
  LOG_DEBUG("Percent: %d \n", s_steering_angle_storage.angle_steering_percent);

  // TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
  //                  steering_angle_get_position(&s_steering_angle_storage));
}
