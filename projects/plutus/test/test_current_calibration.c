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

#define TEST_CURRENT_CALIBRATION_DELAY_SECONDS 5

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

  s_storage.adc_storage = &adc_storage;
}

void teardown_test(void) {}

void test_current_calibration_sample(void) {
  CurrentSenseCalibrationData line = { 0 };

  // Obtain zero point
  LOG_DEBUG("Set current to 0 A\n");
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(current_calibration_sample_point(&s_storage, &line.zero_point, 0));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            line.zero_point.voltage, line.zero_point.current);

  // Obtain max point
  LOG_DEBUG("Set current to %d A\n", TEST_CURRENT_CALIBRATION_MAX);
  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(
      current_calibration_sample_point(&s_storage, &line.max_point, TEST_CURRENT_CALIBRATION_MAX));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
            line.max_point.voltage, line.max_point.current);

  // Test calibration points by obtaining current samples
  CurrentSenseStorage storage = { .line = &line };

  current_sense_init(&storage, &line, s_storage.adc_storage);
  current_sense_register_callback(&storage, prv_callback, NULL);

  delay_s(TEST_CURRENT_CALIBRATION_DELAY_SECONDS);
}
