#include "adc.h"
#include "gpio.h"
#include "unity.h"
#include "log.h"

static GPIOAddress address[] = {
  { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
  { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
  { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 },
  { 2, 2 }, { 2, 3 }, { 2, 4 }, { 2, 5 }
};

static volatile uint8_t s_callback_runs = 0;
static volatile bool s_callback_ran = false;

void prv_callback(ADCChannel adc_channel, uint16_t reading, void *context) {
  s_callback_runs++;
  s_callback_ran = true;
}

void setup_test() {
  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };

  gpio_init();
  interrupt_init();

  for (uint8_t i = 0; i < 16; i++) {
    gpio_init_pin(&address[i], &settings);
  }
}

void test_single() {
  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(10, 1);
  adc_set_channel(11, 1);
  adc_set_channel(12, 1);

  adc_register_callback(10, prv_callback, 0);
  adc_register_callback(11, prv_callback, 0);
  adc_register_callback(12, prv_callback, 0);

  // Background conversions should not be running in single mode
  TEST_ASSERT_EQUAL(0, s_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  adc_read_value(10);
  TEST_ASSERT_EQUAL(3, s_callback_runs);

  // Disable ADC for next test
  adc_disable();
}

void test_continuous() {
  s_callback_ran = false;

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_CONTINUOUS);
  adc_start_continuous();

  // Delay the test so that an interrupt can raise the flag
  LOG_DEBUG("\n");
  TEST_ASSERT_TRUE(s_callback_ran);

  // Disable ADC for continuous test
  adc_disable();
}

void teardown_test(void) { }
