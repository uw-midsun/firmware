#include <stddef.h>
#include <string.h>
#include "nmea_checksum.h"
#include "status.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_nmea_checksum_compute(void) {
  char *message;
  size_t message_len;
  uint8_t computed_checksum = 0;

  message = "$PSRF103,00,01,00,01*";
  message_len = strlen(message);
  TEST_ASSERT(nmea_checksum_compute(message, message_len, &computed_checksum) == STATUS_CODE_OK);

  // 37 dec = 25 hex
  TEST_ASSERT_EQUAL(37, computed_checksum);

  message = "$PSRF103,05,00,01,01*";
  message_len = strlen(message);
  TEST_ASSERT(nmea_checksum_compute(message, message_len, &computed_checksum) == STATUS_CODE_OK);

  // 32 dec = 20 hex
  TEST_ASSERT_EQUAL(32, computed_checksum);

  message = "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64";
  message_len = strlen(message);
  TEST_ASSERT(nmea_checksum_compute(message, message_len, &computed_checksum) == STATUS_CODE_OK);

  // 100 dec = 64 hex
  TEST_ASSERT_EQUAL(100, computed_checksum);

  message = "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52";
  message_len = strlen(message);
  TEST_ASSERT(nmea_checksum_compute(message, message_len, &computed_checksum) == STATUS_CODE_OK);

  // 82 dec = 52 hex
  TEST_ASSERT_EQUAL(82, computed_checksum);

  message = "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52";
  message_len = strlen(message);
  TEST_ASSERT(nmea_checksum_compute(message, message_len, NULL) == STATUS_CODE_INVALID_ARGS);
}

void test_nmea_checksum_validate(void) {
  char *input;
  size_t message_len = 0;

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*64";
  message_len = strlen(input);
  TEST_ASSERT_TRUE(nmea_checksum_validate(input, message_len));

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*z.";
  message_len = strlen(input);
  // should return false because the checksum is not a valid hex integer
  TEST_ASSERT_FALSE(nmea_checksum_validate(input, message_len));

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*00";
  message_len = strlen(input);
  // should return false because checksum is incorrect
  TEST_ASSERT_FALSE(nmea_checksum_validate(input, message_len));

  input =
      "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,"
      "M,15.2,M,,0000*";
  message_len = strlen(input);
  // should return false because the given message does not contain a checksum
  TEST_ASSERT_FALSE(nmea_checksum_validate(input, message_len));
}
