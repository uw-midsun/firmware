#include <inttypes.h>

#include "ltc_current_sense.h"

#include "critical_section.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc2484.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Arbitrary number of testing samples
#define TEST_NUM_SAMPLES 100

static LTCCurrentSenseStorage s_storage;

static uint8_t s_callback_runs = 0;

static void prv_callback(LTCCurrentSenseValue *value, void *context) {
  s_callback_runs++;
  LOG_DEBUG("[%d / %d] | Voltage =  %" PRId32 ", Current =  %" PRId32 "\n", s_callback_runs,
            TEST_NUM_SAMPLES, value->voltage, value->current);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_callback_runs = 0;
}

void teardown_test(void) {}

void test_current_sense(void) {
  // Arbitrary calibration points
  LTCCurrentSenseLineData line = { .zero_point = { 1081, 0 }, .max_point = { 62288, 3000 } };

  // Fill out ADC storage struct and initialize current sense
  LtcAdcStorage adc_storage = {
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_B, 12 },
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 750000,
    .filter_mode = LTC_ADC_FILTER_50HZ_60HZ
  };

  TEST_ASSERT_OK(ltc_current_sense_init(&s_storage, &line, &adc_storage));
  TEST_ASSERT_OK(ltc_current_sense_register_callback(&s_storage, prv_callback, NULL));

  // Wait for samples to accumulate
  while (1) {
    bool disabled = critical_section_start();
    if (s_callback_runs >= TEST_NUM_SAMPLES) {
      return;
    }
    critical_section_end(disabled);
  }
}
