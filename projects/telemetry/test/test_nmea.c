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

void test_nmea_get_gga_sentence_1(void) {
  const char input[] =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  NmeaGgaSentence r = { 0 };
  TEST_ASSERT_TRUE(status_ok(nmea_get_gga_sentence(input, &r)));

  TEST_ASSERT_EQUAL_INT(5, r.time.hh);
  TEST_ASSERT_EQUAL_INT(37, r.time.mm);
  TEST_ASSERT_EQUAL_INT(40, r.time.ss);
  TEST_ASSERT_EQUAL_INT(0, r.time.sss);

  TEST_ASSERT_EQUAL_INT(25, r.latitude.degrees);
  TEST_ASSERT_EQUAL_INT(3, r.latitude.minutes);
  TEST_ASSERT_EQUAL_INT(631, r.latitude.fraction);

  TEST_ASSERT_TRUE(r.north_south == 'N');

  TEST_ASSERT_EQUAL_INT(121, r.longitude.degrees);
  TEST_ASSERT_EQUAL_INT(36, r.longitude.minutes);
  TEST_ASSERT_EQUAL_INT(9, r.longitude.fraction);

  TEST_ASSERT_TRUE(r.east_west == 'E');

  TEST_ASSERT_EQUAL_INT(1, r.position_fix);

  TEST_ASSERT_EQUAL_INT(8, r.satellites_used);

  TEST_ASSERT_EQUAL_INT(1, r.hdop_integer);
  TEST_ASSERT_EQUAL_INT(100, r.hdop_fraction);

  TEST_ASSERT_EQUAL_INT(63, r.msl_altitude_integer);
  TEST_ASSERT_EQUAL_INT(800, r.msl_altitude_fraction);

  TEST_ASSERT_TRUE(r.units_msl_altitude == 'M');

  TEST_ASSERT_EQUAL_INT(15, r.geoid_seperation_integer);
  TEST_ASSERT_EQUAL_INT(200, r.geoid_seperation_fraction);

  TEST_ASSERT_TRUE(r.units_geoid_seperation == 'M');
  TEST_ASSERT_EQUAL_INT(0, r.drs);

  TEST_ASSERT_TRUE(nmea_checksum_validate(input, strlen(input)));
}

void test_nmea_get_gga_sentence_2(void) {
  const char input[] =
      "$GPGGA,001038.00,3334.2313457,N,11211.0576940,W,2,04,5.4,354.682,M,-26.574,M,7.0,0138*79";
  NmeaGgaSentence r = { 0 };
  TEST_ASSERT_TRUE(status_ok(nmea_get_gga_sentence(input, &r)));

  TEST_ASSERT_TRUE(r.time.hh == 0);
  TEST_ASSERT_TRUE(r.time.mm == 10);
  TEST_ASSERT_TRUE(r.time.ss == 38);
  TEST_ASSERT_TRUE(r.time.sss == 0);

  TEST_ASSERT_TRUE(r.latitude.degrees == 33);
  TEST_ASSERT_TRUE(r.latitude.minutes == 34);
  TEST_ASSERT_TRUE(r.latitude.fraction == 231);

  TEST_ASSERT_TRUE(r.north_south == 'N');

  TEST_ASSERT_TRUE(r.longitude.degrees == 112);
  TEST_ASSERT_TRUE(r.longitude.minutes == 11);
  TEST_ASSERT_TRUE(r.longitude.fraction == 57);

  TEST_ASSERT_TRUE(r.east_west == 'W');

  TEST_ASSERT_TRUE(r.position_fix == 2);

  TEST_ASSERT_TRUE(r.satellites_used == 4);

  TEST_ASSERT_TRUE(r.hdop_integer == 5);
  TEST_ASSERT_TRUE(r.hdop_fraction == 400);

  TEST_ASSERT_TRUE(r.msl_altitude_integer == 354);
  TEST_ASSERT_TRUE(r.msl_altitude_fraction == 682);

  TEST_ASSERT_TRUE(r.units_msl_altitude == 'M');

  TEST_ASSERT_TRUE(r.geoid_seperation_integer == -26);
  TEST_ASSERT_TRUE(r.geoid_seperation_fraction == 574);

  TEST_ASSERT_TRUE(r.units_geoid_seperation == 'M');

  // adc placeholder

  TEST_ASSERT_TRUE(r.drs == 138);

  TEST_ASSERT_TRUE(nmea_checksum_validate(input, strlen(input)));
}

void test_nmea_get_vtg_sentence_1(void) {
  const char input[] = "$GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38";

  NmeaVtgSentence vtg = { 0 };

  TEST_ASSERT_TRUE(status_ok(nmea_get_vtg_sentence(input, &vtg)));

  TEST_ASSERT_TRUE(vtg.measure_heading_degrees_integer == 79);
  TEST_ASSERT_TRUE(vtg.measure_heading_degrees_fraction == 650);
  TEST_ASSERT_TRUE(vtg.speed_kmh_integer == 5);
  TEST_ASSERT_TRUE(vtg.speed_kmh_fraction == 0);
}

void test_nmea_get_vtg_sentence_2(void) {
  const char input[] = "$GPVTG,d5,T,,dM,2.69,d0d,K,dA*38";

  NmeaVtgSentence vtg = { 0 };
  TEST_ASSERT_FALSE(status_ok(nmea_get_vtg_sentence(input, &vtg)));
}
