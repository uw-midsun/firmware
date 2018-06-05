#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "nmea.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_gps_nmea_gga(void) {
  const uint8_t input[] =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  TEST_ASSERT_TRUE(nmea_is_gga((char *)input));
  TEST_ASSERT_FALSE(nmea_is_gga(NULL));
  TEST_ASSERT_FALSE(nmea_is_gga("$GPGG"));
}

void test_gps_nmea_vtg(void) {
  const uint8_t input[] = "$GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38";
  TEST_ASSERT_TRUE(nmea_is_vtg((char *)input));
  TEST_ASSERT_FALSE(nmea_is_vtg(NULL));
  TEST_ASSERT_FALSE(nmea_is_vtg("$GPGG"));
}
