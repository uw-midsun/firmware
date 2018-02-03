#include "unity.h"
#include "nmea.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_compute_checksum(void) {
  char* r1 = "25";
  char t1[3];
  nmea_compute_checksum("$PSRF103,00,01,00,01*", t1);
  TEST_ASSERT_EQUAL_STRING(t1, r1);

  char t2[3];
  nmea_compute_checksum("$PSRF103,05,00,01,01*", t2);
  char* r2 = "20";
  TEST_ASSERT_EQUAL_STRING(t2, r2);

  char* r3 = "64";
  char t3[3];
  nmea_compute_checksum(
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64", t3);
  TEST_ASSERT_EQUAL_STRING(r3, t3);

  char* r4 = "52";
  char t4[3];
  nmea_compute_checksum("$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52", t4);
  TEST_ASSERT_EQUAL_STRING(r4, t4);
}

void test_compare_checksum(void) {
  char* input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  TEST_ASSERT_TRUE(nmea_compare_checksum(input));
}
