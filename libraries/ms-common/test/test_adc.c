#include "adc.h"
#include "gpio.h"
#include "unity.h"
#include "log.h"

static GPIOAddress address[] = { { GPIO_PORT_A, 0 }, { GPIO_PORT_A, 1 }, { GPIO_PORT_A, 2 } };

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

  for (uint8_t i = ADC_CHANNEL_0; i < ADC_CHANNEL_2; i++) {
    gpio_init_pin(&address[i], &settings);
  }

  adc_init(ADC_MODE_SINGLE);
}

void teardown_test(void) { }

void test_set_channel(void) {
  // Check that channels can only be set with the correct channel arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_set_channel(NUM_ADC_CHANNEL, 1));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_0, 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_1, 1));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, 1));

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_0, 0));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_1, 0));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_set_channel(ADC_CHANNEL_2, 0));
}

void test_set_callback(void) {
  // Check that callbacks can only be registered with the correct channel arguments
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    adc_register_callback(NUM_ADC_CHANNEL, prv_callback, NULL));

  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL));

  adc_set_channel(ADC_CHANNEL_0, 1);
  adc_set_channel(ADC_CHANNEL_1, 1);
  adc_set_channel(ADC_CHANNEL_2, 1);

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL));
}

void test_single() {
  uint16_t reading;

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(ADC_CHANNEL_0, 1);
  adc_set_channel(ADC_CHANNEL_1, 1);
  adc_set_channel(ADC_CHANNEL_2, 1);

  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL);

  // Callbacks must not run in single mode unless a read occurs
  TEST_ASSERT_FALSE(s_callback_ran);
  TEST_ASSERT_EQUAL(0, s_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_0, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_1, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_read_raw(ADC_CHANNEL_2, &reading));

  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_read_raw(NUM_ADC_CHANNEL, &reading));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, adc_read_raw(ADC_CHANNEL_3, &reading));

  while (!s_callback_ran) {}

  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_TRUE(s_callback_runs > 0);
}

void test_continuous() {
  s_callback_runs = 0;
  s_callback_ran = false;

  // Initialize ADC and check that adc_init() can properly reset the ADC
  adc_init(ADC_MODE_CONTINUOUS);

  adc_set_channel(ADC_CHANNEL_0, 1);
  adc_set_channel(ADC_CHANNEL_1, 1);
  adc_set_channel(ADC_CHANNEL_2, 1);

  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL);

  // Run a busy loop until a callback is triggered
  while (!s_callback_runs) { }

  TEST_ASSERT_TRUE(s_callback_ran);
  TEST_ASSERT_TRUE(s_callback_runs > 0);
}
