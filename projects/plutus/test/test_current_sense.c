#include <inttypes.h>

#include "current_sense.h"

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc2484.h"
#include "ltc_adc.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Arbitrary number of testing samples
#define TEST_CURRENT_SENSE_NUM_SAMPLES 10

// Arbitrary test input voltage
static int32_t s_test_input_voltage = 0;

static volatile uint8_t s_callback_runs = 0;

static CurrentSenseStorage s_storage = { 0 };

static const CurrentSenseCalibrationData s_data = {
  .zero_point = { .voltage = 0, .current = 0 }, .max_point = { .voltage = 1000, .current = 1000000 }
};

static void prv_callback(int32_t *current, void *context) {
  TEST_ASSERT_NOT_EQUAL(NULL, current);
  TEST_ASSERT_EQUAL(*current / 1000, s_test_input_voltage);
  LOG_DEBUG("Current = %" PRId32 "\n", *current);

  s_test_input_voltage += 1000;
  test_ltc_adc_set_input_voltage(s_test_input_voltage);

  s_callback_runs++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_callback_runs = 0;
  s_test_input_voltage = 0;

  LtcAdcSettings adc_settings = {
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 750000,
    .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,
  };

  TEST_ASSERT_OK(current_sense_init(&s_storage, &s_data, &adc_settings));
}

void teardown_test(void) {}

void test_current_sense(void) {
  s_test_input_voltage = 0;
  test_ltc_adc_set_input_voltage(s_test_input_voltage);

  TEST_ASSERT_OK(current_sense_register_callback(&s_storage, prv_callback, NULL));

  // Collect samples and tests that the readings fit the linear relationship defined by the
  // calibration data
  while (s_callback_runs <= TEST_CURRENT_SENSE_NUM_SAMPLES) {
  }
}

void test_current_sense_reset(void) {
  // Obtain the original linear relationship
  int32_t original_slope = (s_data.max_point.voltage - s_data.zero_point.voltage) /
                           (s_data.max_point.current - s_data.zero_point.current);

  // Set an offset by passing in a test input voltage
  test_ltc_adc_set_input_voltage(1000);
  current_sense_zero_reset(&s_storage);

  LOG_DEBUG("Testing with offset = %" PRId32 "\n", s_test_input_voltage);

  // Delay so that the offset appears in the sample data
  delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);

  // Since the input signal is exactly equal to the offset, the current must be equal to zero
  int32_t current;
  bool valid;
  current_sense_get_value(&s_storage, &valid, &current);

  TEST_ASSERT_TRUE(valid);
  TEST_ASSERT_EQUAL(0, current);

  // Test with negative current
  test_ltc_adc_set_input_voltage(900);
  delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);
  current_sense_get_value(&s_storage, &valid, &current);

  TEST_ASSERT_TRUE(valid);
  TEST_ASSERT_TRUE(current < 0);

  current_sense_get_value(&s_storage, &valid, &current);
  TEST_ASSERT_FALSE(valid);

  // Test with positive current
  test_ltc_adc_set_input_voltage(1100);
  delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);
  current_sense_get_value(&s_storage, &valid, &current);

  TEST_ASSERT_TRUE(valid);
  TEST_ASSERT_TRUE(current > 0);

  current_sense_get_value(&s_storage, &valid, &current);
  TEST_ASSERT_FALSE(valid);
}
