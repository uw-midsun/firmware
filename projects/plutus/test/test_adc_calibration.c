#include <inttypes.h>

#include "ltc_adc_calibration.h"

#include "critical_section.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc2484.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Arbitrary number of testing samples
#define TEST_ADC_CALIBRATION_DURATION 25

static LTCCalibrationStorage s_storage = { .storage = { .mosi = { GPIO_PORT_B, 15 },
                                                        .miso = { GPIO_PORT_B, 14 },
                                                        .sclk = { GPIO_PORT_B, 13 },
                                                        .cs = { GPIO_PORT_B, 12 },

                                                        .spi_port = SPI_PORT_2,
                                                        .spi_baudrate = 750000,
                                                        .filter_mode = LTC_ADC_FILTER_50HZ_60HZ },

                                           .value = { 0 } };

static uint8_t s_callback_runs = 0;

void prv_callback(LTCCalibrationValue *value, void *context) {
  s_callback_runs++;
  LOG_DEBUG("[%d / %d] | Voltage =  %" PRId32 ", Current =  %" PRId32 "\n", s_callback_runs,
            TEST_ADC_CALIBRATION_DURATION, value->voltage, value->current);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_callback_runs = 0;
}

void teardown_test(void) {}

void test_adc_calibration(void) {
  // Arbitrary calibration parameters
  LTCCalibrationLineData line = { .zero_point = { 100, 0 }, .max_point = { 20000, 200 } };

  TEST_ASSERT_OK(ltc_adc_calibration_init(&s_storage, &line));
  TEST_ASSERT_OK(ltc_adc_calibration_register_callback(&s_storage, prv_callback, NULL));

  // Wait for samples to accumulate
  while (1) {
    bool disabled = critical_section_start();
    if (s_callback_runs >= TEST_ADC_CALIBRATION_DURATION) {
      return;
    }
    critical_section_end(disabled);
  }
}
