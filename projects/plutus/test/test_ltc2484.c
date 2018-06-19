#include <stddef.h>
#include <stdint.h>
#include "ltc2484.h"
#include "test_helpers.h"
#include "unity.h"

StatusCode TEST_MOCK(ltc2484_raw_adc_to_uv)(uint8_t *spi_data, int32_t *voltage) {
  Ltc2484Response tmp;

  tmp.u8data[0] = spi_data[3];
  tmp.u8data[1] = spi_data[2];
  tmp.u8data[2] = spi_data[1];
  tmp.u8data[3] = spi_data[0];

  // We have an overrange condition when the SIG and MSB bits are both set
  // (see Table 3 on p.16 of the datasheet)
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_OVERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  // We have an underrange condition when the SIG and MSB bits are not set and
  // B27-25 are set
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_UNDERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Sign extend tmp.i32data as two's complement
  // We don't care about the following values, so we can shift them out:
  // - The higher 3 bits are the SIG, DMY and MSB
  // - The lower 4 bits are the configuration word from the previous conversion
  int32_t adc_value = ((tmp.i32data << 3) >> 8);

  // Convert the voltage to uV
  // 2^24 * 4092 * 1000 = 6.86523679e13 (which fits in an int64_t)
  // dividing by 2^24 gives 4092000, which fits in an int32_t
  *voltage = (int32_t)(((int64_t)(adc_value)*LTC2484_V_REF_MILLIVOLTS * 1000) >> 24);

  return STATUS_CODE_OK;
}

void setup_test(void) {}

void teardown_test(void) {}

void test_parse_data_negative(void) {
  // 00011111 10011011 10000000 10011100
  // 0x1f     0x9b     0x80     0x9c
  //
  // Step 1: Throw away the first 3 and last 5 bits
  // xxx11111 10011011 10000000 100xxxxx
  //
  // Step 2: Truncate the last byte
  // 11111100 11011100 00000100
  //
  // Step 3: Convert to two's complement
  // 0b111111001101110000000100 => -205820 in two's complement
  //
  // Step 4: Convert the raw ADC value to the voltage using the ADC equation
  // (-205820 * (4092 * 1000)) / 2^24 = -50200
  uint8_t spi_response[4] = { 0x1f, 0x9b, 0x80, 0x9c };
  int32_t result = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltc2484_raw_adc_to_uv(spi_response, &result));
  TEST_ASSERT_EQUAL(-50200, result);
}

void test_parse_data_positive(void) {
  // 00100000 01101010 00010111 11001100
  // 0x20     0x6a     0x17     0xcc
  //
  // Step 1: Throw away the first 3 and last 5 bits
  // xxx00000 01101010 00010111 110xxxxx
  //
  // Step 2: Truncate the last byte
  // 00000011 01010000 10111110
  //
  // Step 3: Convert to two's complement
  // 0b000000110101000010111110 => 217278 in two's complement
  //
  // Step 4: Convert the raw ADC value to the voltage using the ADC equation
  // (217278 * (4092 * 1000)) / 2^24 = 52994
  uint8_t spi_response[4] = { 0x20, 0x6a, 0x17, 0xcc };
  int32_t result = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltc2484_raw_adc_to_uv(spi_response, &result));
  TEST_ASSERT_EQUAL(52994, result);
}

void test_parse_data_overrange(void) {
  // According to Table 3 (p.16 in the datasheet) an overrange condition occurs
  // when SIG and MSB are set in the first byte
  uint8_t spi_response[4] = { 0x30, 0x00, 0x00, 0x00 };
  int32_t result = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE, ltc2484_raw_adc_to_uv(spi_response, &result));
}

void test_parse_data_not_overrange(void) {
  // 00111000 00000000 00000000 00000000
  // 0x38     0x00     0x00     0x00
  //
  // Step 1: Throw away the first 3 and last 5 bits
  // xxx11000 00000000 00000000 000xxxxx
  //
  // Step 2: Truncate the last byte
  // 11000000 00000000 00000000
  //
  // Step 3: Convert to two's complement
  // 0b110000000000000000000000 => -4194304 in two's complement
  //
  // Step 4: Convert the raw ADC value to the voltage using the ADC equation
  // (-4194304 * (4092 * 1000)) / 2^24 = -1023000
  uint8_t spi_response[4] = { 0x38, 0x00, 0x00, 0x00 };
  int32_t result = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OK, ltc2484_raw_adc_to_uv(spi_response, &result));
  TEST_ASSERT_EQUAL(-1023000, result);
}

void test_parse_data_underrange(void) {
  // According to Table 3 (p.16 in the datasheet) an underrange condition
  // occurs when SIG, MSB are not set and B27-25 are set
  uint8_t spi_response[4] = { 0x0f, 0x00, 0x00, 0x00 };
  int32_t result = 0;

  TEST_ASSERT_EQUAL(STATUS_CODE_OUT_OF_RANGE, ltc2484_raw_adc_to_uv(spi_response, &result));
}
