#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "nmea.h"
#include "test_helpers.h"
#include "unity.h"
#include "util.h"
#include "gps.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_gps_nmea_gga(void) {
  const uint8_t input[] =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  evm_gps_gga_sentence r = evm_gps_parse_nmea_gga_sentence(input, SIZEOF_ARRAY(input));
  // Just chose a random thing to test. Not extensive yet
  LOG_DEBUG("r.north_south: %s\n", (char *)&r.north_south);
  TEST_ASSERT_TRUE(r.time.hh == 5);
  TEST_ASSERT_TRUE(r.time.mm == 37);
  TEST_ASSERT_TRUE(r.time.ss == 40);
  TEST_ASSERT_TRUE(r.time.sss == 0);

  TEST_ASSERT_TRUE(r.latitude.degrees == 25);
  TEST_ASSERT_TRUE(r.latitude.minutes == 3);
  TEST_ASSERT_TRUE(r.latitude.fraction == 6319);

  TEST_ASSERT_TRUE(r.north_south == (uint8_t)'N');

  TEST_ASSERT_TRUE(r.longtitude.degrees == 121);
  TEST_ASSERT_TRUE(r.longtitude.minutes == 36);
  TEST_ASSERT_TRUE(r.longtitude.fraction == 99);

  TEST_ASSERT_TRUE(r.east_west == (uint8_t)'E');

  TEST_ASSERT_TRUE(r.position_fix == 1);

  TEST_ASSERT_TRUE(r.satellites_used == 8);

  TEST_ASSERT_TRUE(r.hdop_1 == 1);
  TEST_ASSERT_TRUE(r.hdop_2 == 1);

  TEST_ASSERT_TRUE(r.msl_altitude_1 == 63);
  TEST_ASSERT_TRUE(r.msl_altitude_2 == 8);

  TEST_ASSERT_TRUE(r.units_msl_altitude == (uint8_t)'M');

  TEST_ASSERT_TRUE(r.geoid_seperation_1 == 15);
  TEST_ASSERT_TRUE(r.geoid_seperation_2 == 2);

  TEST_ASSERT_TRUE(r.units_geoid_seperation == (uint8_t)'M');

  TEST_ASSERT_TRUE(evm_gps_compare_checksum((char *)input));
}

void test_disable(void) {
  char message[24];
  disable_message_type(5, message);
  TEST_ASSERT_EQUAL_STRING("$PSRF103,05,00,00,01*21", message);
}
