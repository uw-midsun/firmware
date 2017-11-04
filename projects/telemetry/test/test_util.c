#include "util.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_compute_checksum(void) {
  TEST_ASSERT_EQUAL_STRING("25", compute_checksum("$PSRF103,00,01,00,01*"));
  TEST_ASSERT_EQUAL_STRING("20", compute_checksum("$PSRF103,05,00,01,01*"));
  TEST_ASSERT_EQUAL_STRING("64", compute_checksum("$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64"));
  TEST_ASSERT_EQUAL_STRING("52", compute_checksum("$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52"));
}
