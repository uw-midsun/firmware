#include <inttypes.h>

#include "ltc_adc_calibration.h"

#include "delay.h"
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

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_adc_calibration(void) {
  // Arbitrary calibration parameters
  LTCCalibrationLineData line = { .zero_point = { 100, 0 }, .max_point = { 20000, 200 } };
  LTCCalibrationValue value = { 0 };

  TEST_ASSERT_OK(ltc_adc_calibration_init(&s_storage, &line));

  // Wait for samples to accumulate
  for (uint8_t i = 0; i < TEST_ADC_CALIBRATION_DURATION; i++) {
    ltc_adc_calibration_get_value(&s_storage, &value);

    LOG_DEBUG("[%d / %d] | Voltage =  %" PRId32 ", Current =  %" PRId32 "\n", i + 1,
              TEST_ADC_CALIBRATION_DURATION, value.voltage, value.current);

    // Delay for the conversion time specified in ltc2484 config file
    delay_ms(LTC2484_MAX_CONVERSION_TIME_MS);
  }
}
