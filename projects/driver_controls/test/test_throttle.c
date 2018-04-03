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

#define TEST_THROTTLE_ADC_CHANNEL_MAIN ADS1015_CHANNEL_0
#define TEST_THROTTLE_ADC_CHANNEL_SECONDARY ADS1015_CHANNEL_1
#define THROTTLE_UPDATE_PERIOD_US (1000 * THROTTLE_UPDATE_PERIOD_MS)

static Ads1015Storage s_ads1015_storage;
static ThrottleStorage s_throttle_storage;
static ThrottleCalibrationData s_calibration_data;

// Readings that are used as fake inputs to throttle for both channels.
static int16_t s_mocked_reading_main;
static int16_t s_mocked_reading_secondary;

static const ThrottleZoneThreshold s_threshes_main[NUM_THROTTLE_ZONES] = {
  { .min = 320, .max = 683 },   // Brake zone
  { .min = 684, .max = 1064 },  // Coast zone
  { .min = 1065, .max = 1410 }  // Accel zone
};
static const ThrottleLine s_line[NUM_THROTTLE_CHANNELS] = {
  { .full_brake_reading = 325, .full_throttle_reading = 1405 },  // Main channel
  { .full_brake_reading = 160, .full_throttle_reading = 710 }    //  Secondary channel
};
#define TEST_THROTTLE_TOLERANCE 10

// Mocks ads1015_read_raw to allow feeding desired inputs to throttle for both channels.
StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  if (channel == TEST_THROTTLE_ADC_CHANNEL_MAIN) {
    *reading = s_mocked_reading_main;
  } else {
    *reading = s_mocked_reading_secondary;
  }
  return STATUS_CODE_OK;
}

// Initializes calibration data from static data.
static void prv_set_calibration_data(ThrottleCalibrationData *data) {
  memcpy(data->zone_thresholds_main, s_threshes_main,
         NUM_THROTTLE_ZONES * sizeof(ThrottleZoneThreshold));
  memcpy(data->line, s_line, NUM_THROTTLE_CHANNELS * sizeof(ThrottleLine));
  data->tolerance = TEST_THROTTLE_TOLERANCE;
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
  event_queue_init();
  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  prv_set_calibration_data(&s_calibration_data);
  throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                TEST_THROTTLE_ADC_CHANNEL_MAIN, TEST_THROTTLE_ADC_CHANNEL_SECONDARY);
}

void teardown_test(void) {}

void test_throttle_init_invalid_args(void) {
  // Test with valid arguments.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_OK,
      throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                    TEST_THROTTLE_ADC_CHANNEL_MAIN, TEST_THROTTLE_ADC_CHANNEL_SECONDARY));
  // Check for null pointers.
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      throttle_init(NULL, &s_calibration_data, &s_ads1015_storage, TEST_THROTTLE_ADC_CHANNEL_MAIN,
                    TEST_THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      throttle_init(&s_throttle_storage, NULL, &s_ads1015_storage, TEST_THROTTLE_ADC_CHANNEL_MAIN,
                    TEST_THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(
      STATUS_CODE_INVALID_ARGS,
      throttle_init(&s_throttle_storage, &s_calibration_data, NULL, TEST_THROTTLE_ADC_CHANNEL_MAIN,
                    TEST_THROTTLE_ADC_CHANNEL_SECONDARY));
  // Check for invalid channels.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                                  NUM_ADS1015_CHANNELS, TEST_THROTTLE_ADC_CHANNEL_SECONDARY));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    throttle_init(&s_throttle_storage, &s_calibration_data, &s_ads1015_storage,
                                  TEST_THROTTLE_ADC_CHANNEL_MAIN, NUM_ADS1015_CHANNELS));
}

void test_throttle_get_pos_invalid_args(void) {
  ThrottlePosition position;
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(NULL, &position));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, throttle_get_position(&s_throttle_storage, NULL));
}

void test_throttle_reading_in_brake_zone(void) {
  ThrottlePosition position;
  Event e;

  // Middle of brake zone.
  s_mocked_reading_main =
      (s_threshes_main[THROTTLE_ZONE_BRAKE].max + s_threshes_main[THROTTLE_ZONE_BRAKE].min) / 2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_BRAKE, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_BRAKE, e.id);
}

void test_throttle_reading_in_coast_zone(void) {
  ThrottlePosition position;
  Event e;

  // Middle of coast zone.
  s_mocked_reading_main =
      (s_threshes_main[THROTTLE_ZONE_COAST].max + s_threshes_main[THROTTLE_ZONE_COAST].min) / 2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_COAST, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_COAST, e.id);
}

void test_throttle_reading_in_accel_zone(void) {
  ThrottlePosition position;
  Event e;

  // Middle of acceleration zone.
  s_mocked_reading_main =
      (s_threshes_main[THROTTLE_ZONE_ACCEL].max + s_threshes_main[THROTTLE_ZONE_ACCEL].min) / 2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_ACCEL, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_ACCEL, e.id);
}

void test_throttle_reading_at_bottom_thresh_valid(void) {
  ThrottlePosition position;
  Event e;

  // At the very bottom of the brake zone.
  s_mocked_reading_main = s_threshes_main[THROTTLE_ZONE_BRAKE].min;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_BRAKE, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_BRAKE, e.id);
}

void test_throttle_reading_at_bottom_thresh_invalid(void) {
  ThrottlePosition position;
  Event e;

  // Right below the brake zone and out of bound.
  s_mocked_reading_main = s_threshes_main[THROTTLE_ZONE_BRAKE].min - 1;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_reading_at_top_thresh_valid(void) {
  ThrottlePosition position;
  Event e;

  // At the very top of accel zone.
  s_mocked_reading_main = s_threshes_main[THROTTLE_ZONE_ACCEL].max;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_ACCEL, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_ACCEL, e.id);
}

void test_throttle_reading_at_top_thresh_invalid(void) {
  ThrottlePosition position;
  Event e;

  // Just above the accel zone and not within bounds.
  s_mocked_reading_main = s_threshes_main[THROTTLE_ZONE_ACCEL].max + 1;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_secondary_reading_on_edge_valid(void) {
  ThrottlePosition position;
  Event e;
  // The main reading is chosen to be the midpoint of its line and secondary reading is tolerance
  // units away from its midpoint which hits the edges of the band around the secondary line.
  // This position happens to be in coast zone.

  // Secondary reading exactly on the upper edge.
  s_mocked_reading_main = (s_line[THROTTLE_CHANNEL_MAIN].full_throttle_reading +
                           s_line[THROTTLE_CHANNEL_MAIN].full_brake_reading) /
                          2;
  s_mocked_reading_secondary = (s_line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading +
                                s_line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading) /
                                   2 +
                               TEST_THROTTLE_TOLERANCE;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_COAST, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_COAST, e.id);

  // Secondary reading exactly on the lower edge.
  s_mocked_reading_main = (s_line[THROTTLE_CHANNEL_MAIN].full_throttle_reading +
                           s_line[THROTTLE_CHANNEL_MAIN].full_brake_reading) /
                          2;
  s_mocked_reading_secondary = (s_line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading +
                                s_line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading) /
                                   2 -
                               TEST_THROTTLE_TOLERANCE;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_OK(throttle_get_position(&s_throttle_storage, &position));
  TEST_ASSERT_EQUAL(THROTTLE_ZONE_COAST, position.zone);
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_COAST, e.id);
}

void test_throttle_secondary_reading_on_edge_invalid(void) {
  ThrottlePosition position;
  Event e;
  // The main reading is chosen to be the midpoint of its line and secondary reading is
  // tolerance + 1 units away from the midpoint which just passes the edges of the band
  // around the secondary line.
  // This position happens to be in coast zone.

  // Secondary reading one unit above the upper edge.
  s_mocked_reading_main = (s_line[THROTTLE_CHANNEL_MAIN].full_throttle_reading +
                           s_line[THROTTLE_CHANNEL_MAIN].full_brake_reading) /
                          2;
  s_mocked_reading_secondary = (s_line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading +
                                s_line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading) /
                                   2 +
                               TEST_THROTTLE_TOLERANCE + 1;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);

  // Secondary reading one unit below the lower edge.
  s_mocked_reading_main = (s_line[THROTTLE_CHANNEL_MAIN].full_throttle_reading +
                           s_line[THROTTLE_CHANNEL_MAIN].full_brake_reading) /
                          2;
  s_mocked_reading_secondary = (s_line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading +
                                s_line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading) /
                                   2 -
                               TEST_THROTTLE_TOLERANCE - 1;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_out_of_bound(void) {
  ThrottlePosition position;
  Event e;

  // Out of bound case.
  s_mocked_reading_main = s_threshes_main[THROTTLE_ZONE_ACCEL].max * 2;
  s_mocked_reading_secondary = s_mocked_reading_main / 2;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_out_of_sync(void) {
  ThrottlePosition position;
  Event e;

  // Readings out of sync case.
  s_mocked_reading_main =
      (s_threshes_main[THROTTLE_ZONE_ACCEL].max + s_threshes_main[THROTTLE_ZONE_ACCEL].min) / 2;
  s_mocked_reading_secondary = s_mocked_reading_main / 3;
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_stale_main_channel(void) {
  ThrottlePosition position;
  Event e;

  // Turning off main channel to produce the stale reading case.
  ads1015_configure_channel(&s_ads1015_storage, TEST_THROTTLE_ADC_CHANNEL_MAIN, false, NULL, NULL);
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}

void test_throttle_event_fault_stale_secondary_channel(void) {
  ThrottlePosition position;
  Event e;

  // Turning off second channel to produce the stale reading case.
  ads1015_configure_channel(&s_ads1015_storage, TEST_THROTTLE_ADC_CHANNEL_SECONDARY, false, NULL,
                            NULL);
  delay_us(THROTTLE_UPDATE_PERIOD_US);
  TEST_ASSERT_EQUAL(STATUS_CODE_TIMEOUT, throttle_get_position(&s_throttle_storage, &position));
  event_process(&e);
  TEST_ASSERT_EQUAL(INPUT_EVENT_PEDAL_FAULT, e.id);
}
