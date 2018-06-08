#include <inttypes.h>

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

#define TEST_CURRENT_CALIBRATION_DELAY_SECONDS 10

static void prv_callback(int32_t current, void *context) {}

static LtcAdcStorage adc_storage = {
  .mosi = { GPIO_PORT_B, 15 },
  .miso = { GPIO_PORT_B, 14 },
  .sclk = { GPIO_PORT_B, 13 },
  .cs = { GPIO_PORT_B, 12 },
  .spi_port = SPI_PORT_2,
  .spi_baudrate = 750000,
  .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,
};

static CurrentCalibrationStorage s_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_current_calibration_sample(void) {
  CurrentSenseCalibrationData data = { 0 };

  // Reset calibration and obtain zero point
  TEST_ASSERT_OK(current_calibration_init(&s_storage, &adc_storage));
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
      current_calibration_sample_point(&s_storage, &line.max_point, TEST_CURRENT_CALIBRATION_MAX));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            data.max_point.voltage, data.max_point.current);
}
