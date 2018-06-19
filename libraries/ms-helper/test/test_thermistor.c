#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "thermistor.h"
#include "unity.h"

#define TEMPERATURE_TOLERANCE 50

StatusCode TEST_MOCK(adc_read_converted)(ADCChannel adc_channel, uint16_t *reading) {
  switch (adc_channel) {
    case (ADC_CHANNEL_1):
      *reading = 1500;
      break;
    case (
        ADC_CHANNEL_2):  // Lookup table will return the lowest resistance. Translates to 0 degrees
      *reading = 806;
      break;
    case (ADC_CHANNEL_3):  // Lookup table will return the highest resistance. translates to 100
                           // degrees
      *reading = 2733;
      break;
    case (ADC_CHANNEL_4):
      *reading = 3000;
      break;
    case (ADC_CHANNEL_REF):
      *reading = 3000;
      break;
    default:
      *reading = 1500;
      break;
  }
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_get_channel)(GPIOAddress address, ADCChannel *adc_channel) {
  *adc_channel = address.pin;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(adc_set_channel)(ADCChannel adc_channel, bool new_state) {
  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

// Tests the thermistor at a standard temperature(25 degrees)
void test_thermistor_normal(void) {
  GPIOAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 1,
  };
  ThermistorStorage storage;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, 10000));

  uint32_t reading = 0;
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  TEST_ASSERT_UINT32_WITHIN(TEMPERATURE_TOLERANCE, 25000, reading);
  LOG_DEBUG("Temperature: %u\n", reading);
}

// Tests the lower bound of the resistance-temperature lookup table (0 deg)
void test_thermistor_min_temp(void) {
  GPIOAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 2,
  };
  ThermistorStorage storage;

  uint32_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, 10000));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %u\n", reading);
  TEST_ASSERT_UINT32_WITHIN(TEMPERATURE_TOLERANCE, 0, reading);
}

// Tests the upper bound of the resistance-temperature lookup table (100 deg)
void test_thermistor_max_temp(void) {
  GPIOAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 3,
  };
  ThermistorStorage storage;

  uint32_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, 10000));
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %u\n", reading);
  TEST_ASSERT_UINT32_WITHIN(TEMPERATURE_TOLERANCE, 100000, reading);
}

// Tests for a temperature exceeding the lookup tables ranges
void test_thermistor_out_of_range(void) {
  GPIOAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 4,
  };
  ThermistorStorage storage;

  uint32_t reading = 0;
  TEST_ASSERT_OK(thermistor_init(&storage, gpio_addr, 10000));
  TEST_ASSERT_NOT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %u\n", reading);
}
