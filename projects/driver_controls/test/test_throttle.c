#include <string.h>
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "test_helpers.h"
#include "throttle.h"
#include "unity.h"

#define THROTTLE_ADC_CHANNEL_MAIN ADS1015_CHANNEL_0
#define THROTTLE_ADC_CHANNEL_SECONDARY ADS1015_CHANNEL_1
#define THROTTLE_UPDATE_PERIOD_US (1000 * THROTTLE_UPDATE_PERIOD_MS)
static Ads1015Storage s_ads1015_storage;
static ThrottleStorage s_throttle_storage;
static ThrottleCalibrationData s_calibration_data;
static int16_t s_mocked_reading_main;
static int16_t s_mocked_reading_secondary;
static int16_t s_threshes_main[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES] = {
  { 320, 683 },   // Brake zone
  { 683, 1064 },  // Coast zone
  { 1046, 1410 }  // Accel zone
};
static int16_t s_line_of_best_fit[NUM_THROTTLE_CHANNELS][NUM_THROTTLE_THRESHES] = {
  { 325, 1405 },  // Main channel
  { 160, 710 }    //  Secondary channel
};
static int16_t s_tolerance_secondary = 10;

StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  if (channel == THROTTLE_ADC_CHANNEL_MAIN) {
    *reading = s_mocked_reading_main;
  } else {
    *reading = s_mocked_reading_secondary;
  }
  return STATUS_CODE_OK;
}

// Sets zone thresholds for the given channel.
static void prv_set_calibration_data(int16_t threshes[NUM_THROTTLE_ZONES][NUM_THROTTLE_THRESHES],
                                     ThrottleCalibrationData *data) {
  for (ThrottleZone zone = THROTTLE_ZONE_BRAKE; zone < NUM_THROTTLE_ZONES; zone++) {
    for (ThrottleThresh thresh = THROTTLE_THRESH_MIN; thresh < NUM_THROTTLE_THRESHES; thresh++) {
      data->zone_thresholds_main[zone][thresh] = threshes[zone][thresh];
    }
  }
  for (ThrottleChannel channel = THROTTLE_CHANNEL_MAIN; channel < NUM_THROTTLE_CHANNELS;
       channel++) {
    for (ThrottleThresh thresh = THROTTLE_THRESH_MIN; thresh < NUM_THROTTLE_THRESHES; thresh++) {
      data->line_of_best_fit[channel][thresh] = s_line_of_best_fit[channel][thresh];
    }
  }
  data->tolerance = s_tolerance_secondary;
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
  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  prv_set_calibration_data(s_threshes_main, &s_calibration_data);
}

void teardown_test(void) {}

void test_throttle_init_invalid_args(void) {
  // Test with valid arguments.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                                  THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY));
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(NULL, &s_calibration_data, &s_ads1015_storage,
                                  THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, NULL, &s_ads1015_storage,
                                  THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, &s_calibration_data, NULL,
                                  THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY));
  // Check for invalid channels.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                                  NUM_ADS1015_CHANNELS, THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                                  THROTTLE_ADC_CHANNEL_MAIN, NUM_ADS1015_CHANNELS));
}

void test_throttle_get_pos_invalid_args(void) {
  ThrottlePosition position;
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(NULL, &position));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(&s_throttle_storage, NULL));
}

void test_throttle_verify_event_brake(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Brake zone.
  s_mocked_reading_main = (s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MAX] +
                           s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_BRAKE][THROTTLE_THRESH_MIN]) /
                          2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_BRAKE, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_BRAKE, e.id);
}

void test_throttle_verify_event_coast(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Coast zone.
  s_mocked_reading_main = (s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_COAST][THROTTLE_THRESH_MAX] +
                           s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_COAST][THROTTLE_THRESH_MIN]) /
                          2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_COAST, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_COAST, e.id);
}

void test_throttle_verify_event_accel(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Acceleration zone.
  s_mocked_reading_main = (s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] +
                           s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MIN]) /
                          2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_ACCEL, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_PRESSED, e.id);
}

void test_throttle_event_fault_out_of_bound(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Out of bound case.
  s_mocked_reading_main = s_throttle_storage.calibration_data
                              ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] *
                          2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_out_of_sync(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Readings out of sync case.
  s_mocked_reading_main = (s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MAX] +
                           s_throttle_storage.calibration_data
                               ->zone_thresholds_main[THROTTLE_ZONE_ACCEL][THROTTLE_THRESH_MIN]) /
                          2;
  s_mocked_reading_secondary = s_mocked_reading_main / 3;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_stale_channel_main(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Turning off main channel to produce the stale reading case.
  ads1015_configure_channel(&s_ads1015_storage, THROTTLE_ADC_CHANNEL_MAIN, false, NULL, NULL);
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_stale_channel_secondary(void) {
  ThrottlePosition position;
  Event e;
  event_queue_init();
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                THROTTLE_ADC_CHANNEL_MAIN, THROTTLE_ADC_CHANNEL_SECONDARY);
  // Turning off second channel to produce the stale reading case.
  ads1015_configure_channel(&s_ads1015_storage, THROTTLE_ADC_CHANNEL_SECONDARY, false, NULL, NULL);
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}
