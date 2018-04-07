#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include "ltc_adc.h"

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "plutus_config.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_ADC_NUM_SAMPLES 200

typedef struct {
  int32_t min_value;
  int32_t max_value;
  uint32_t samples;
} TestAdcStorage;

static void prv_adc_callback(int32_t *value, void *context) {
  TestAdcStorage *storage = (TestAdcStorage *)context;

  storage->min_value = MIN(*value, storage->min_value);
  storage->max_value = MAX(*value, storage->max_value);

  storage->samples++;
  if ((storage->samples + 1) % 10 == 0) {
    LOG_DEBUG("[%lu/" STRINGIFY(TEST_ADC_NUM_SAMPLES) "] Samples taken\n", (storage->samples + 1));
  }
}

static volatile TestAdcStorage s_storage = {
  .min_value = INT32_MAX,
  .max_value = INT32_MIN,
  .samples = 0,
};

static LtcAdcStorage s_adc_settings = {
  .mosi = { GPIO_PORT_B, 15 },  //
  .miso = { GPIO_PORT_B, 14 },  //
  .sclk = { GPIO_PORT_B, 13 },  //
  .cs = { GPIO_PORT_B, 12 },    //

  .spi_port = SPI_PORT_2,  //
  .spi_baudrate = 750000,  //
  .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  s_storage.min_value = INT32_MAX;
  s_storage.max_value = INT32_MIN;
  s_storage.samples = 0;
}

void teardown_test(void) {}

void test_ltc_adc_characterize_ripple(void) {
  TEST_ASSERT_OK(ltc_adc_init(&s_adc_settings));
  TEST_ASSERT_OK(ltc_adc_register_callback(&s_adc_settings, prv_adc_callback, (void *)&s_storage));

  while (s_storage.samples < TEST_ADC_NUM_SAMPLES) {
    // block until TEST_ADC_NUM_SAMPLES samples have been taken
  }

  LOG_DEBUG("Min: %" PRId32 "\n", s_storage.min_value);
  LOG_DEBUG("Max: %" PRId32 "\n", s_storage.max_value);
  LOG_DEBUG("Ripple: %" PRId32 "\n", s_storage.max_value - s_storage.min_value);
}
