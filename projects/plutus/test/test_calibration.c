#include "ltc_calibration.h"

#include <inttypes.h>

#include "ltc_current_sense.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "delay.h"
#include "test_helpers.h"
#include "unity.h"
#include "log.h"

#define TEST_MAX_CURRENT_POINT    3
#define TEST_SAMPLE_DELAY_SECONDS 10

static void prv_callback(LTCCurrentSenseValue *value, void *context) {
  LOG_DEBUG("Voltage =  %" PRId32 ", Current =  %" PRId32 "\n", value->voltage, value->current);
}

static LTCCalibrationStorage s_storage = {
  .adc_storage = {
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 750000,
    .filter_mode = LTC_ADC_FILTER_50HZ_60HZ
  }
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_calibration_sample(void) {
  LTCCurrentSenseLineData line = { 0 };

  // Obtain zero point  
  LOG_DEBUG("Set current to 0 A\n");
  delay_s(TEST_SAMPLE_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(ltc_calibration_sample_point(&s_storage, &line.zero_point, 0));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
             line.zero_point.voltage, line.zero_point.current);

  // Obtain max point
  LOG_DEBUG("Set current to %d A\n", TEST_MAX_CURRENT_POINT);
  delay_s(TEST_SAMPLE_DELAY_SECONDS);
  LOG_DEBUG("Start sampling\n");
  TEST_ASSERT_OK(ltc_calibration_sample_point(&s_storage, &line.max_point, TEST_MAX_CURRENT_POINT));
  LOG_DEBUG("Sampling finished -> { Voltage = %" PRId32 ", Current = %" PRId32 " }\n",
             line.max_point.voltage, line.max_point.current);

  // Test calibration points by obtaining current samples
  LTCCurrentSenseStorage storage = { .line = &line };

  ltc_current_sense_init(&storage, &line, &s_storage.adc_storage);
  ltc_current_sense_register_callback(&storage, prv_callback, NULL);

  delay_s(TEST_SAMPLE_DELAY_SECONDS);
}