#include "cobs.h"
#include "unity.h"
#include "log.h"

void setup_test(void) {}
void teardown_test(void) {}

void test_cobs_encode(void) {
  uint8_t data[] = { 0x1, 0x2, 0x0, 0x4, 0x5, 0x8, 0x0, 0x0, 0xFF, 0xF5 };
  uint8_t encoded_data[COBS_MAX_ENCODED_LEN(SIZEOF_ARRAY(data))];

  size_t encoded_len = SIZEOF_ARRAY(encoded_data);
  cobs_encode(data, SIZEOF_ARRAY(data), encoded_data, &encoded_len);

  LOG_DEBUG("Original %ld bytes, encoded %ld\n", SIZEOF_ARRAY(data), encoded_len);
  for (size_t i = 0; i < encoded_len; i++) {
    printf("%ld: 0x%x\n", i, encoded_data[i]);
  }
}
