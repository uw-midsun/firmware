#include <stdint.h>
#include <stddef.h>
#include "unity.h"
#include "test_helpers.h"
#include "crc15.h"

void setup_test(void) {
  crc15_init_table();
}

void teardown_test(void) { }

void test_crc15_calculate_example() {
  // example from table 24
  uint8_t data[2] = { 0x00, 0x01 };
  TEST_ASSERT_EQUAL(0x3D6E, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_rdcva() {
  // datasheet p.55
  uint8_t data[2] = { 0x00, 0x04 };
  TEST_ASSERT_EQUAL(0x07C2, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_adcv() {
  // datasheet p.55
  uint8_t data[2] = { 0x03, 0x70 };
  TEST_ASSERT_EQUAL(0xAF42, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_clrcell() {
  // datasheet p.55
  uint8_t data[] = { 0x07, 0x11 };
  TEST_ASSERT_EQUAL(0xC9C0, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_pladc() {
  // datasheet p.55
  uint8_t data[] = { 0x9F, 0x14 };
  TEST_ASSERT_EQUAL(0x1C48, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_wrcomm() {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x21 };
  TEST_ASSERT_EQUAL(0x24B2, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

/*
void test_crc15_calculate_wrcomm_i2c_slave_result() {
  // datasheet p.56
  uint8_t data[] = { 0x6A, 0x08, 0x00, 0x18, 0x0A, 0xA9 };
  TEST_ASSERT_EQUAL(0x6DFB, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}
*/

void test_crc15_calculate_wrcomm_slave_result() {
  // datasheet p.56
  uint8_t data[] = { 0x85, 0x50, 0x8A, 0xA0, 0x8C, 0xC9 };
  TEST_ASSERT_EQUAL(0x89A4, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_stcomm() {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x23 };
  TEST_ASSERT_EQUAL(0xB9E4, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_rdcomm() {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x22 };
  TEST_ASSERT_EQUAL(0x32D6, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_rdcomm_i2c_data_result() {
  // datasheet p.56
  uint8_t data[] = { 0x6A, 0x07, 0x70, 0x17, 0x7A, 0xA1 };
  TEST_ASSERT_EQUAL(0xD0DE, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}

void test_crc15_calculate_rdcomm_slave_result() {
  // datasheet p.57
  uint8_t data[] = { 0x75, 0x5F, 0x7A, 0xAF, 0x7C, 0xCF };
  TEST_ASSERT_EQUAL(0xF2BA, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}
