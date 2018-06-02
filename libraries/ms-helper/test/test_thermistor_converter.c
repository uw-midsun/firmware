#include "log.h"
#include "test_helpers.h"
#include "thermistor_converter.h"
#include <math.h>

/*
static int prv_voltage_to_resistance(uint32_t vout) {
 int vo = (int)vout;
  int r2 = (10000000/vo)*3000 - 10000000;
  int r = (int)vout;
  int a = 8984; 
  int b = 2495;
  int c = 2;

  int temp = a +b*log(r) + c*log(r)*log(r)*log(r);
  int x1 = a +b*log(r);
  printf("%d  %d  %d  %d \t", a, (int)(a+b*log(r)), (int)(c*log(r)*log(r)*log(r)), (int)(1 / temp));
  return 1 / temp;
} */



void setup_test(void) {}

void teardown_test(void) {}

void test_thermistor_init(void) {
  TEST_ASSERT_OK(thermistor_converter_init());
}

void test_thermistor_values(void) {
  uint32_t reading;
  while (true) {
    reading = thermistor_converter_get_temp();
    printf("Temperature reading: %lu\n", reading);
  }
}
