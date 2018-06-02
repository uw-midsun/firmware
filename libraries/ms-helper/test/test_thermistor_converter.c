#include <math.h>
#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "test_helpers.h"
#include "thermistor_converter.h"

/*
static int prv_voltage_to_resistance(uint32_t vout) {
 int vo = (int)vout;
  int r2 = (10000000/vo)*3000 - 10000000;
  int r = (int)vout;
  int a = 8984;
  int b = 2495;yy
  int c = 2;

  int temp = a +b*log(r) + c*log(r)*log(r)*log(r);
  int x1 = a +b*log(r);
  printf("%d  %d  %d  %d \t", a, (int)(a+b*log(r)), (int)(c*log(r)*log(r)*log(r)), (int)(1 / temp));
  return 1 / temp;
} */

void setup_test(void) {}

void teardown_test(void) {}

// void test_thermistor_init(void) {
  // // TEST_ASSERT_OK(thermistor_converter_init());
  // GPIOSettings settings = {
  //   .direction = GPIO_DIR_IN,
  //   .state = GPIO_STATE_LOW,
  //   .resistor = GPIO_RES_NONE,
  //   .alt_function = GPIO_ALTFN_ANALOG,
  // };

  // GPIOAddress address = {
  //   .port = GPIO_PORT_A,
  //   .pin = 4
  // };

  // // initialize gpio
  // gpio_init();
  // gpio_init_pin(&address, &settings);

  // // initialize interrupts
  // interrupt_init();

  // // soft_timer_init();

  // // initialize the channel
  // adc_init(ADC_MODE_CONTINUOUS);
  // adc_set_channel(ADC_CHANNEL_4, true);

  // return STATUS_CODE_OK;
// }

void test_thermistor_values(void) {
  ThermistorStorage storage = {
    .sibling_resistance = 10000,
    .source_voltage = 3000,
    .channel = ADC_CHANNEL_4,
  };

  // TEST_ASSERT_OK(thermistor_converter_init());
  GPIOSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GPIOAddress address = { .port = GPIO_PORT_A, .pin = 4 };

  // initialize gpio
  gpio_init();
  gpio_init_pin(&address, &settings);

  // initialize interrupts
  interrupt_init();

  soft_timer_init();

  // initialize the channel
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(storage.channel, true);

  uint32_t reading;
  while (true) {
    reading = thermistor_converter_get_temp(&storage);
    printf("Temperature reading: %lu\n", reading);
  }
}
