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
  .zero_point = { .voltage = 0, .current = 0 }, .max_point = { .voltage = 1000, .current = 10 }
};

static void prv_callback(int32_t current, void *context) {
  TEST_ASSERT_EQUAL(current * 100, s_test_input_voltage);
  LOG_DEBUG("Current = %" PRId32 "\n", current);

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

  for (int i = 0; i < TEST_CURRENT_SENSE_NUM_SAMPLES; i++) {
    // Send in new test voltage and start a reset
    s_test_input_voltage = (i + 1) * 1000;
    test_ltc_adc_set_input_voltage(s_test_input_voltage);

    LOG_DEBUG("Testing with offset = %" PRId32 "\n", s_test_input_voltage);

    // Delay so that the offset appears in the sample data
    delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);
    current_sense_zero_reset(&s_storage);

    // Start a new cycle with the input voltage being equal to the offset. A current of zero
    // should be seen
    delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);

    int32_t current = 0;
    current_sense_get_value(&s_storage, &current);
    TEST_ASSERT_EQUAL(0, current);

    // Obtain new linear relationship
    int32_t new_slope = (s_data.max_point.voltage - s_data.zero_point.voltage) /
                        (s_data.max_point.current - s_data.zero_point.current);

    // Ensure that the new set of points maintain their relationship
    TEST_ASSERT_EQUAL(original_slope, new_slope);
  }
}
