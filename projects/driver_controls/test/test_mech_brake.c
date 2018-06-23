
#include <stdio.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "mech_brake.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static MechBrakeStorage s_mech_brake_storage;
static Ads1015Storage s_ads1015_storage;

#define TEST_MECH_BRAKE_TOLERANCE 2

static MechBrakeCalibrationData s_calib_data = {
  .zero_value = 336,
  .hundred_value = 730,
};

// input reading used as a fake input
static int16_t s_mocked_reading;

const MechBrakeSettings brake_settings = {
  .brake_pressed_threshold = 500,
  .tolerance = 2,
  .channel = ADS1015_CHANNEL_2,
  .ads1015 = &s_ads1015_storage,
};

// Mocks ads1015_read_raw
StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  *reading = s_mocked_reading;
  return STATUS_CODE_OK;
}

void setup_test() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };

  i2c_init(I2C_PORT_1, &i2c_settings);
  event_queue_init();

  GPIOAddress ready_pin = { .port = GPIO_PORT_A, .pin = 10 };

  ads1015_init(&s_ads1015_storage, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data);
}

void teardown_test(void) {}

void test_mech_brake_init_invalid_args(void) {
  // Test with valid arguments.
  TEST_ASSERT_EQUAL(STATUS_CODE_OK,
                    mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  // Check for null pointers on each parameter.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mech_brake_init(NULL, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mech_brake_init(&s_mech_brake_storage, NULL, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mech_brake_init(&s_mech_brake_storage, &brake_settings, NULL));
}

void test_mech_brake_get_percentage_invalid_args(void) {
  int16_t percentage;
  // Check for null pointers.
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, mech_brake_get_percentage(NULL, &percentage));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    mech_brake_get_percentage(&s_mech_brake_storage, NULL));
}

void test_mech_brake_percentage_in_released_zone(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = 400;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, e.id);
}

void test_mech_brake_percentage_in_pressed_zone(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = 600;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, e.id);
}

void test_mech_brake_percentage_below_bounds(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = -1000;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
                    mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
}

void test_mech_brake_percentage_above_bounds(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = 2 << 12;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE,
                    mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
}

void test_mech_brake_percentage_within_lower_tolerance(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = -80;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_MECHANICAL_BRAKE_RELEASED, e.id);
}

void test_mech_brake_percentage_within_upper_tolerance(void) {
  int16_t percentage = 0;

  s_calib_data.zero_value = 0;
  s_calib_data.hundred_value = 1 << 12;

  s_mocked_reading = 4176;
  Event e;

  TEST_ASSERT_OK(mech_brake_init(&s_mech_brake_storage, &brake_settings, &s_calib_data));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, mech_brake_get_percentage(&s_mech_brake_storage, &percentage));
  delay_ms(5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, e.id);
}
