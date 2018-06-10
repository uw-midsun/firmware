#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
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

  TEST_ASSERT_OK(thermistor_init(&storage, &settings));

  uint32_t reading = 0;
  TEST_ASSERT_OK(thermistor_get_temp(&storage, &reading));
  LOG_DEBUG("Temperature: %lu\n", reading);
}
