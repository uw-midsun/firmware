#include <inttypes.h>

// Sample routine to demonstrate usage of the current calibration module

#include "current_calibration.h"
#include "current_sense.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// The module does not know what current the adc readings correspond to, so keep an arbitrary
// max point for testing
#define TEST_CURRENT_CALIBRATION_MAX 3

#define TEST_CURRENT_CALIBRATION_DELAY_SECONDS 1

static LtcAdcSettings adc_settings = {
  .mosi = { GPIO_PORT_B, 15 },
  .miso = { GPIO_PORT_B, 14 },
  .sclk = { GPIO_PORT_B, 13 },
  .cs = { GPIO_PORT_B, 12 },
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 750000,
  .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_current_calibration_sample(void) {
  CurrentCalibrationStorage s_storage = { 0 };
  CurrentSenseCalibrationData data = { 0 };
  LtcAdcStorage adc_storage = { 0 };

  // Reset calibration and obtain zero point
  TEST_ASSERT_OK(current_calibration_init(&s_storage, &adc_storage, &adc_settings));
  LOG_DEBUG("Set current to 0 A\n");
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(current_calibration_sample_point(&s_storage, &data.zero_point, 0));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            data.zero_point.voltage, data.zero_point.current);

  // Reset calibration and obtain max point
  LOG_DEBUG("Set current to %d A\n", TEST_CURRENT_CALIBRATION_MAX);
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(
      current_calibration_sample_point(&s_storage, &data.max_point, TEST_CURRENT_CALIBRATION_MAX));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            data.max_point.voltage, data.max_point.current);
}

void test_current_calibration_reset(void) {
  CurrentCalibrationStorage s_storage = { 0 };
  CurrentSenseCalibrationData data = { 0 };
  LtcAdcStorage adc_storage = { 0 };

  int32_t test_offset = 100;
  adc_storage.context = &test_offset;

  // Reset calibration and obtain zero point
  TEST_ASSERT_OK(current_calibration_init(&s_storage, &adc_storage, &adc_settings));
  LOG_DEBUG("Sampling zero point after reset #1 with test offset %" PRId32 "\n", test_offset);
  TEST_ASSERT_OK(current_calibration_sample_point(&s_storage, &data.zero_point, 0));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            data.zero_point.voltage, data.zero_point.current);
  TEST_ASSERT_EQUAL(test_offset, data.zero_point.voltage);

  uint16_t test_average = test_offset;

  // Use test offsets increasing by 100
  for (uint8_t i = 2; i < 11; i++) {
    test_offset = i * 100;
    adc_storage.context = &test_offset;

    // Average formula for consectutive hundreds
    if (i <= 10) {
      test_average = ((100 * (i + 1)) / 2);
    } else {
      uint8_t n = i - CURRENT_CALIBRATION_OFFSET_WINDOW;
      test_average = ((100 * (i * (i + 1) - n * (n + 1))) / 2 / CURRENT_CALIBRATION_OFFSET_WINDOW);
    }

    LOG_DEBUG("Sampling zero point after reset #%d with test offset %" PRId32 "\n", i + 2,
              test_offset);
    TEST_ASSERT_OK(current_calibration_zero_reset(&s_storage, &data.zero_point));
    LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
              data.zero_point.voltage, data.zero_point.current);
    TEST_ASSERT_EQUAL(test_average, data.zero_point.voltage);
  }
}
