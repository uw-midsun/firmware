#include <stddef.h>
#include <string.h>
#include "nmea.h"
#include "unity.h"
#include "status.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_compute_checksum(void) {
  char *message;
  size_t message_len;
  char computed_checksum[3];

  message = "$PSRF103,00,01,00,01*";
  message_len = strlen(message);
  TEST_ASSERT(nmea_compute_checksum(message, message_len, computed_checksum, sizeof(computed_checksum)) == STATUS_CODE_OK);
  TEST_ASSERT_EQUAL_STRING("25", computed_checksum);

  message = "$PSRF103,05,00,01,01*";
  message_len = strlen(message);
  TEST_ASSERT(nmea_compute_checksum(message, message_len, computed_checksum, sizeof(computed_checksum)) == STATUS_CODE_OK);
  TEST_ASSERT_EQUAL_STRING("20", computed_checksum);

  message = "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64";
  message_len = strlen(message);
  TEST_ASSERT(nmea_compute_checksum(message, message_len, computed_checksum, sizeof(computed_checksum)) == STATUS_CODE_OK);
  TEST_ASSERT_EQUAL_STRING("64", computed_checksum);

  message = "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52";
  message_len = strlen(message);
  TEST_ASSERT(nmea_compute_checksum(message, message_len, computed_checksum, sizeof(computed_checksum)) == STATUS_CODE_OK);
  TEST_ASSERT_EQUAL_STRING("52", computed_checksum);

  message = "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52";
  message_len = strlen(message);
  char small_out[2];
  TEST_ASSERT(nmea_compute_checksum(message, message_len, small_out, sizeof(small_out)) == STATUS_CODE_INVALID_ARGS);
}

void test_compare_checksum(void) {
  char *input;
  size_t message_len;

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  message_len = strlen(input) + 1;
  TEST_ASSERT_TRUE(nmea_compare_checksum(input, message_len));

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*00";
  message_len = strlen(input) + 1;
  // should return false because checksum is incorrect
  TEST_ASSERT_FALSE(nmea_compare_checksum(input, message_len));

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*";
  message_len = strlen(input) + 1;
  // should return false because the given message does not contain a checksum
  TEST_ASSERT_FALSE(nmea_compare_checksum(input, message_len));
}
