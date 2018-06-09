#include "log.h"
#include "test_helpers.h"
#include "thermistor.h"

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
    .adc_channel = ADC_CHANNEL_4,
  };

  ThermistorStorage storage;

  // initialize gpio pin
  gpio_init_pin(&gpio_addr, &gpio_settings);

  // initialize the channel
  adc_init(ADC_MODE_CONTINUOUS);

  // for some reason this test is being included in x86 tests
  // and causing the build to fail
  // TEST_ASSERT_OK(thermistor_init(&storage, &settings));

  // uint16_t reading = thermistor_get_temp(&storage);
  // printf("Temperature: %u\n", reading);
}
