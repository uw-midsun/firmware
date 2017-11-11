#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "nmea.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_gps_nmea_gga(void) {
  uint8_t input[] =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  NMEAResult r = parse_nmea_sentence(input, sizeof(input) / sizeof(input[0]));
  // Just chose a random thing to test. Not extensive yet
  TEST_ASSERT_TRUE(r.gga.north_south == (uint8_t)'N');
}
