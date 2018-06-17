#include <inttypes.h>

#include "current_sense.h"

#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc2484.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Arbitrary number of testing samples
#define TEST_NUM_SAMPLES 5

// Arbitrary test input voltage
static int32_t s_test_input_voltage = 0;

static volatile uint8_t s_callback_runs = 0;

static CurrentSenseStorage s_storage = { 0 };
static LtcAdcSettings s_adc_settings = { 0 };
static LtcAdcStorage s_adc_storage = { 0 };

static CurrentSenseCalibrationData s_data = { .zero_point = { .voltage = 0, .current = 0 },
                                              .max_point = { .voltage = 1000, .current = 10 } };

static void prv_callback(int32_t current, void *context) {
  TEST_ASSERT_EQUAL(current * 100, s_test_input_voltage);
  LOG_DEBUG("Current = %" PRId32 "\n", current);

  s_callback_runs++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_callback_runs = 0;
  s_test_input_voltage = 0;

  s_adc_settings = (LtcAdcSettings){ .mosi = { GPIO_PORT_B, 15 },
                                     .miso = { GPIO_PORT_B, 14 },
                                     .sclk = { GPIO_PORT_B, 13 },
                                     .cs = { GPIO_PORT_B, 12 },
                                     .spi_port = SPI_PORT_2,
                                     .spi_baudrate = 750000,
                                     .filter_mode = LTC_ADC_FILTER_50HZ_60HZ };

  s_adc_storage = (LtcAdcStorage){ 0 };
  s_adc_storage.context = &s_test_input_voltage;
}

void teardown_test(void) {}

void test_current_sense(void) {
  s_test_input_voltage = 1000;

  TEST_ASSERT_OK(current_sense_init(&s_storage, &s_data, &s_adc_storage, &s_adc_settings));
  TEST_ASSERT_OK(current_sense_register_callback(&s_storage, prv_callback, NULL));

  // Wait for samples to accumulate
  while (s_callback_runs <= TEST_NUM_SAMPLES) {
  }
}

void test_current_sense_reset(void) {
  // Obtain the original linear relationship
  int32_t original_slope = (s_data.max_point.voltage - s_data.zero_point.voltage) /
                           (s_data.max_point.current - s_data.zero_point.current);

  TEST_ASSERT_OK(current_sense_init(&s_storage, &s_data, &s_adc_storage, &s_adc_settings));

  for (int i = 0; i < 5; i++) {
    // Send in new test voltage and start a reset
    s_test_input_voltage = (i + 1) * 1000;

    LOG_DEBUG("Testing with offset = %" PRId32 "\n", s_test_input_voltage);
    current_sense_zero_reset(&s_storage);
    printf("Zero = { %" PRId32 " V, %" PRId32 " A }, Max = { %" PRId32 " V, %" PRId32 " A }\n",
           s_data.zero_point.voltage, s_data.zero_point.current, s_data.max_point.voltage,
           s_data.max_point.current);

    TEST_ASSERT_EQUAL(s_test_input_voltage, s_data.zero_point.voltage);

    // Obtain new linear relationship
    int32_t new_slope = (s_data.max_point.voltage - s_data.zero_point.voltage) /
                        (s_data.max_point.current - s_data.zero_point.current);

    // Ensure that the new set of points maintain their relationship
    TEST_ASSERT_EQUAL(original_slope, new_slope);
  }
}
