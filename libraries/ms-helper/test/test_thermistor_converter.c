#include "log.h"
#include "test_helpers.h"
#include "thermistor_converter.h"

/*static int prv_voltage_to_resistance(uint16_t vout) {
  int vo = (int)vout / 1000;
  int r2 = 30000/vo - 10000;
  return r2;
} */

void setup_test(void) {}

void teardown_test(void) {}

void test_thermistor_init(void) {
  TEST_ASSERT_OK(thermistor_converter_init());
}

void test_thermistor_values(void) {
  uint16_t reading = thermistor_converter_get_temp();
  while (true) {
    printf("Voltage reading: %u\tCalculated Resistance:\n", reading);
  }
}
