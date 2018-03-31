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

LtcAdcStorage adc_settings = {
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
}

void teardown_test(void) {}

void test_ltc_adc_characterize_ripple(void) {
  ltc_adc_init(&adc_settings);

  int32_t value = 0;
  StatusCode status = ltc_adc_get_value(&adc_settings, &value);

  int32_t min_value = value;
  int32_t max_value = value;
  for (int readings = 0; readings < TEST_ADC_NUM_SAMPLES; ++readings) {
    // Delay to ensure that conversions have run
    delay_ms(200);

    status = ltc_adc_get_value(&adc_settings, &value);
    if (status_ok(status)) {
      min_value = MIN(value, min_value);
      max_value = MAX(value, max_value);
    } else {
      LOG_DEBUG("ERROR: The status was %d\n", status);
    }

    if (readings % 10 == 0) {
      LOG_DEBUG("[%d/200] Samples taken\n", readings);
    }
  }

  LOG_DEBUG("Min: %" PRId32 "\n", min_value);
  LOG_DEBUG("Max: %" PRId32 "\n", max_value);
  LOG_DEBUG("Ripple: %" PRId32 "\n", max_value - min_value);
}
