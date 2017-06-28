#include "adc.h"
#include "gpio.h"
#include "unity.h"
#include "log.h"
#include "critical_section.h"

static GPIOAddress address[] = { { GPIO_PORT_A, 0 }, { GPIO_PORT_A, 1 }, { GPIO_PORT_A, 2 } };

static volatile uint8_t s_callback_runs = 0;
static volatile bool s_callback_ran = false;

void prv_callback(ADCChannel adc_channel, uint16_t reading, void *context) {
  LOG_DEBUG("ADC Channel %d with reading %d\n", adc_channel, reading);
  s_callback_runs++;
  s_callback_ran = true;
}

void setup_test() {
  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };

  gpio_init();
  interrupt_init();

  for (uint8_t i = ADC_CHANNEL_0; i < ADC_CHANNEL_2; i++) {
    gpio_init_pin(&address[i], &settings);
  }
}

void teardown_test(void) { }

void test_single() {
  uint16_t reading;

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  // Check that channels can only be set with the correct channel arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(NUM_ADC_CHANNEL, 1));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_0, 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_1, 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, 1));

  // Check that callbacks can only be registered with the correct channel arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(NUM_ADC_CHANNEL, 1));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL));

  // Background conversions should not be running in single mode
  TEST_ASSERT_EQUAL(0, s_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_value(ADC_CHANNEL_0, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_read_value(NUM_ADC_CHANNEL, &reading));

  TEST_ASSERT_TRUE(0 <= reading && reading <= 4096);
  TEST_ASSERT_EQUAL(3, s_callback_runs);
}

void test_continuous() {
  ADCChannel adc_channel;

  s_callback_runs = 0;
  s_callback_ran = false;

  // Disable all but one of the channels and check that it only works with correct arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(NUM_ADC_CHANNEL, 0));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, 0));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_0,
    prv_callback,
    &adc_channel));

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_CONTINUOUS);

  bool critical_section = critical_section_start();
  TEST_ASSERT_TRUE(critical_section);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_trigger_callback(ADC_CHANNEL_0));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_trigger_callback(ADC_CHANNEL_1));

  TEST_ASSERT_EQUAL(2, s_callback_runs);

  critical_section_end(true);
  s_callback_runs = 0;

  TEST_ASSERT_TRUE(s_callback_runs > 0);
}
