#include "log.h"
#include "test_helpers.h"
#include "thermistor_converter.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_thermistor(void) {
  GPIOAddress gpio_addr = {
    .port = GPIO_PORT_A,
    .pin = 4,
  };

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  ThermistorSettings settings = {
    .sibling_resistance = 10000000,
    .source_voltage = 3000,
    .context = 0,
    .gpio_settings = &gpio_settings,
    .gpio_addr = &gpio_addr,
    .adc_mode = ADC_MODE_CONTINUOUS,
    .adc_channel = ADC_CHANNEL_4,
  };

  ThermistorStorage storage;

  TEST_ASSERT_OK(thermistor_converter_init(&storage, &settings));

  while (true) {
    uint16_t reading = thermistor_converter_get_temp(&storage);
    printf("Temperature: %u\n", reading);
  }
}
