#include <string.h>
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "throttle.h"
#include "unity.h"

static Ads1015Storage ads1015_storage;
static ThrottleStorage throttle_storage;
static ThrottleCalibrationData calibration_data;
static int16_t s_mocked_reading;
static EventID s_mocked_event;
static int16_t s_threshes_main[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES] = {
  { 325, 625 },   //
  { 625, 1135 },  //
  { 1135, 1404 }  //
};
static int16_t s_threshes_secondary[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES] = {
  { 162, 312 },  //
  { 312, 575 },  //
  { 575, 707 }   //
};
static int16_t s_tolerance = 10;

StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  if (channel == throttle_storage.channel_main) {
    *reading = s_mocked_reading;
  } else {
    *reading = s_mocked_reading / 2;
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(event_raise)(EventID id, uint16_t data){
  s_mocked_event = id;
}


// Sets the tolerance for comparing channel readings.
static void prv_set_calibration_data_tolerance(int16_t tolerance, ThrottleCalibrationData *data) {
  data->channel_readings_tolerance = tolerance;
}

// Sets zone thresholds for the given channel.
static void prv_set_calibration_data(ThrottleChannel channel,
                                     int16_t threshes[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES],
                                     ThrottleCalibrationData *data) {
  for (ThrottleZone zone = THROTTLE_ZONE_BRAKE; zone < NUM_THROTTLE_ZONES; zone++) {
    for (ThrottleThresh thresh = THROTTLE_THRESH_MIN; thresh < NUM_THROTTLE_THRESHES; thresh++) {
      data->zone_thresholds[channel][zone][thresh] = threshes[zone][thresh];
    }
  }
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
  prv_set_calibration_data(THROTTLE_CHANNEL_MAIN, s_threshes_main, &calibration_data);
  prv_set_calibration_data(THROTTLE_CHANNEL_SECONDARY, s_threshes_secondary, &calibration_data);
  prv_set_calibration_data_tolerance(s_tolerance, &calibration_data);
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
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(NULL, &position));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(&throttle_storage, NULL));
}

void test_throttle_verify_zone_event(void){
  ThrottlePosition position;
  throttle_init(&throttle_storage, &calibration_data, &ads1015_storage, ADS1015_CHANNEL_0,
                ADS1015_CHANNEL_1);
  // Brake zone.
  s_mocked_reading =
      (throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MAX] +
       throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN]) /
      2;
  delay_us(100);
  TEST_ASSERT_OK(throttle_get_position(&throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_BRAKE, position.zone);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_BRAKE, s_mocked_event);

  // Coast zone.
  s_mocked_reading =
      (throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MAX] +
       throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_COAST][THROTTLE_THRESH_MIN]) /
      2;
  delay_us(100);
  TEST_ASSERT_OK(throttle_get_position(&throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_COAST, position.zone);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_COAST, s_mocked_event);

  // Acceleration zone.
  s_mocked_reading =
      (throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] +
       throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MIN]) /
      2;
  delay_us(100);
  TEST_ASSERT_OK(throttle_get_position(&throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_ACCEL, position.zone);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_PRESSED, s_mocked_event);

  // Out of bound case.
  s_mocked_reading =
      throttle_storage.calibration_data
           ->zone_thresholds[THROTTLE_CHANNEL_MAIN][THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] * 2;
  delay_us(100);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&throttle_storage, &position));
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, s_mocked_event);
}