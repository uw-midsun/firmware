#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "interrupt.h"
#include "log.h"
#include "nmea.h"
#include "nmea_checksum.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_gps_nmea_gga(void) {
  const uint8_t input[] =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  nmea_gga_sentence r = { 0 };
  TEST_ASSERT_TRUE(status_ok(nmea_get_gga_sentence(input, strlen((char *)input), &r)));
  TEST_ASSERT_TRUE(r.time.hh == 5);
  TEST_ASSERT_TRUE(r.time.mm == 37);
  TEST_ASSERT_TRUE(r.time.ss == 40);
  TEST_ASSERT_TRUE(r.time.sss == 0);

  TEST_ASSERT_TRUE(r.latitude.degrees == 25);
  TEST_ASSERT_TRUE(r.latitude.minutes == 3);
  TEST_ASSERT_TRUE(r.latitude.fraction == 6319);

  TEST_ASSERT_TRUE(r.north_south == (uint8_t)'N');

  TEST_ASSERT_TRUE(r.longitude.degrees == 121);
  TEST_ASSERT_TRUE(r.longitude.minutes == 36);
  TEST_ASSERT_TRUE(r.longitude.fraction == 99);

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

  TEST_ASSERT_TRUE(nmea_checksum_validate((char *)input, strlen((char *)input)));
}

void test_gps_nmea_vtg(void) {
  const uint8_t input[] = "$GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38";

  nmea_vtg_sentence vtg = { 0 };
  TEST_ASSERT_TRUE(status_ok(nmea_get_vtg_sentence(input, strlen((char *)input), &vtg)));

  TEST_ASSERT_TRUE(vtg.degrees_1 == 79);
  TEST_ASSERT_TRUE(vtg.degrees_2 == 65);
  TEST_ASSERT_TRUE(vtg.speed_kmh_1 == 5);
  TEST_ASSERT_TRUE(vtg.speed_kmh_2 == 0);
}

void test_fail_vtg(void) {
  const uint8_t input[] = "$GPVTG,d5,T,,dM,2.69,d0d,K,dA*38";

  nmea_vtg_sentence vtg = { 0 };
  TEST_ASSERT_FALSE(status_ok(nmea_get_vtg_sentence(input, strlen((char *)input), &vtg)));
}
