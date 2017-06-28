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

  for (uint8_t i = 0; i < 16; i++) {
    gpio_init_pin(&address[i], &settings);
  }
}

void teardown_test(void) { }

void test_single() {
  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(ADC_CHANNEL_0, 1);
  adc_set_channel(ADC_CHANNEL_1, 1);
  adc_set_channel(ADC_CHANNEL_2, 1);

  adc_register_callback(ADC_CHANNEL_0, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_1, prv_callback, NULL);
  adc_register_callback(ADC_CHANNEL_2, prv_callback, NULL);

  // Background conversions should not be running in single mode
  TEST_ASSERT_EQUAL(0, s_callback_runs);

  // Ensure that the conversions happen once adc_read_value is called
  uint16_t reading = adc_read_value(10);

  TEST_ASSERT_TRUE(0 <= reading && reading <= 4096);
  TEST_ASSERT_EQUAL(3, s_callback_runs);
}

void test_continuous() {
  s_callback_ran = false;

  // Initialize the ADC to single mode and configure the channels
  adc_init(ADC_MODE_CONTINUOUS);

  // Delay the test so that an interrupt can raise the flag
  TEST_ASSERT_TRUE(s_callback_ran);
}
