#include "can_unpack_impl.h"

#include <stdint.h>

#include "can_msg.h"
#include "misc.h"
#include "unity.h"

static const CANMessage s_msg = {
  .data_u8 = { 32, 64, 29, 76, 56, 21, 3, 1 },
};

void setup_test(void) {}

void teardown_test(void) {}

void test_can_unpack_impl_u8(void) {
  uint8_t f[8] = { 0 };
  can_unpack_impl_u8(&s_msg, &f[0], &f[1], &f[2], &f[3], &f[4], &f[5], &f[6], &f[7]);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_msg.data_u8, f, SIZEOF_ARRAY(s_msg.data_u8));
  can_unpack_impl_u8(&s_msg, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                     NULL);  // Ensure it doesn't segfault.
}

void test_can_unpack_impl_u16(void) {
  uint16_t f[4] = { 0 };
  can_unpack_impl_u16(&s_msg, &f[0], &f[1], &f[2], &f[3]);
  TEST_ASSERT_EQUAL_UINT16_ARRAY(s_msg.data_u16, f, SIZEOF_ARRAY(s_msg.data_u16));
  can_unpack_impl_u16(&s_msg, NULL, NULL, NULL, NULL);  // Ensure it doesn't segfault.
}

void test_can_unpack_impl_u32(void) {
  uint32_t f[2] = { 0 };
  can_unpack_impl_u32(&s_msg, &f[0], &f[1]);
  TEST_ASSERT_EQUAL_UINT32_ARRAY(s_msg.data_u32, f, SIZEOF_ARRAY(s_msg.data_u32));
  can_unpack_impl_u32(&s_msg, NULL, NULL);  // Ensure it doesn't segfault.
}

void test_can_unpack_impl_u64(void) {
  uint64_t f = 0;
  can_unpack_impl_u64(&s_msg, &f);
  TEST_ASSERT_EQUAL_UINT64(s_msg.data, f);
  can_unpack_impl_u64(&s_msg, NULL);  // Ensure it doesn't segfault.
}
