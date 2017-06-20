#include "adc.h"
#include "gpio.h"
#include "unity.h"
#include <stdio.h>

static GPIOAddress address[] = { { 0, 0 }, { 0, 1 }, { 0, 2 } };
static GPIOAddress invalid_address[] = { { 0, 8 }, { 0, 9 }, { 0, 10 } };

void setup_test() {
  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };
  
  gpio_init();
  gpio_init_pin(&address, &settings);
}

void test_single() {
  adc_init(ADC_MODE_SINGLE);
  adc_init_pin(&address, ADC_SAMPLE_RATE_1);
  uint16_t reading;

  // Ensure that converted values are within the required range
  for (uint8_t i = 0; i < 8; i++) {
    reading = adc_read(&address, 3000);
    TEST_ASSERT_TRUE((reading >= 0) && (reading <= 3000));
  }
  
  // Read directly from the register. Should stay constant despite changes with the input
  // (Analog input should be connected for this test)
  
  
  for (uint32_t i = 0; i < 500; i++) {
    LOG_DEBUG("Single Read #%d = %d\n", i, reading, 3000*ADC1->DR/4095);
  }
  

  // Disable ADC for continuous test
  adc_disable();
}

void test_continuous() {
  adc_init(ADC_MODE_CONTINUOUS);
  adc_init_pin(&address, ADC_SAMPLE_RATE_1);
  uint16_t reading;

  // Ensure that converted values are within the required range
  for (uint8_t i = 0; i < 8; i++) {
    reading = adc_read(&address, 3000);
    TEST_ASSERT_TRUE((reading >= 0) && (reading <= 4096));
  }

}

void test_valid() {
  // Ensure that sampling rates can only be set for pins mapped to an ADC channel

  for (uint8_t i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL(STATUS_CODE_OK, adc_init_pin(&address[i], ADC_SAMPLE_RATE_1));
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, adc_init_pin(&invalid_address[i], ADC_SAMPLE_RATE_1));
  }

}

void teardown_test(void) { }

