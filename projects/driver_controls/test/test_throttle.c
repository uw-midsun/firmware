#include <string.h>
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "throttle.h"
#include "unity.h"

static Ads1015Storage ads1015_storage;
static ThrottleStorage throttle_storage;
static ThrottleCalibrationData calibration_data;

// This will be way of setting calibration data before the calibration routine is implemented.
static void prv_set_calibration_data(ThrottleCalibrationData *data, int16_t min, int16_t brake_max,
                                     int16_t coast_max, int16_t accel_max, int16_t tolerance) {
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN] = min * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MAX] =
      brake_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MIN] =
      brake_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MAX] =
      coast_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MIN] =
      coast_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] =
      accel_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ALL][THROTTLE_THRESH_MIN] = min * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ALL][THROTTLE_THRESH_MAX] =
      accel_max * 2;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN] = min;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MAX] =
      brake_max;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MIN] =
      brake_max;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MAX] =
      coast_max;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MIN] =
      coast_max;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] =
      accel_max;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_ALL][THROTTLE_THRESH_MIN] = min;
  data->zone_thresholds[THROTTLE_CHANNEL_SECONDARY][THROTTLE_ZONE_ALL][THROTTLE_THRESH_MAX] =
      accel_max;
  data->channel_readings_tolerance = tolerance;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(TEST_ADS1015_I2C_PORT, &i2c_settings);
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  prv_set_calibration_data(&calibration_data, 0, 25, 50, 75, 0);
}

void teardown_test(void) {}

void test_throttle_init_invalid_args(void) {
  // Test with valid arguments.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    throttle_init(&throttle_storage, &calibration_data, &ads1015_storage,
                                  ADS1015_CHANNEL_0, ADS1015_CHANNEL_1));
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(NULL, &calibration_data, &ads1015_storage, ADS1015_CHANNEL_0,
                                  ADS1015_CHANNEL_1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&throttle_storage, NULL, &ads1015_storage, ADS1015_CHANNEL_0,
                                  ADS1015_CHANNEL_1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&throttle_storage, &calibration_data, NULL, ADS1015_CHANNEL_0,
                                  ADS1015_CHANNEL_1));
  // Check for invalid channels.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&throttle_storage, &calibration_data, &ads1015_storage,
                                  NUM_ADS1015_CHANNELS, ADS1015_CHANNEL_1));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&throttle_storage, &calibration_data, &ads1015_storage,
                                  ADS1015_CHANNEL_0, NUM_ADS1015_CHANNELS));
}

void test_throttle_get_pos_invalid_args(void) {
  ThrottlePosition position;
  throttle_init(&throttle_storage, &calibration_data, &ads1015_storage, ADS1015_CHANNEL_0,
                ADS1015_CHANNEL_1);
  delay_ms(50);
  // Test with valid arguments.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, throttle_get_position(&throttle_storage, &position));
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(NULL, &position));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(&throttle_storage, NULL));
}
