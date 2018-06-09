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
#define TEST_NUM_SAMPLES 25
#define TEST_INPUT_VOLTAGE 1000

static volatile uint8_t s_callback_runs = 0;

static CurrentSenseStorage s_storage = { 0 };
static CurrentSenseCalibrationData s_data = { .zero_point = { 888, 0 },
                                              .max_point = { 62304, 3000 } };

static void prv_callback(int32_t current, void *context) {
  s_callback_runs++;

  int32_t x_min = s_data.zero_point.voltage;
  int32_t y_min = s_data.zero_point.current;
  int32_t x_max = s_data.max_point.voltage;
  int32_t y_max = s_data.max_point.current;

  int32_t test_current = (y_max) * (TEST_INPUT_VOLTAGE) / (x_max - x_min);

  TEST_ASSERT_EQUAL(current, test_current);

  LOG_DEBUG("Current = %" PRId32 "\n", current);
}

// Mock the adc readings
StatusCode TEST_MOCK(ltc2484_raw_adc_to_uv)(uint8_t *spi_data, int32_t *voltage) {
  *voltage = TEST_INPUT_VOLTAGE;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_callback_runs = 0;
}

void teardown_test(void) {}

void test_current_sense(void) {
  // Fill out ADC storage struct and initialize current sense
  LtcAdcSettings adc_settings = { .mosi = { GPIO_PORT_B, 15 },
                                  .miso = { GPIO_PORT_B, 14 },
                                  .sclk = { GPIO_PORT_B, 13 },
                                  .cs = { GPIO_PORT_B, 12 },
                                  .spi_port = SPI_PORT_2,
                                  .spi_baudrate = 750000,
                                  .filter_mode = LTC_ADC_FILTER_50HZ_60HZ };

  LtcAdcStorage adc_storage = { 0 };

  TEST_ASSERT_OK(current_sense_init(&s_storage, &s_data, &adc_storage, &adc_settings));
  TEST_ASSERT_OK(current_sense_register_callback(&s_storage, prv_callback, NULL));

  // Wait for samples to accumulate
  while (s_callback_runs <= TEST_NUM_SAMPLES) {
  }
}
