#include "fifo.h"
#include "unity.h"
#include "test_helpers.h"
#include "log.h"

#define TEST_FIFO_BUFFER_LEN 11
#define TEST_FIFO_OFFSET 0x123

static Fifo s_fifo;
static uint32_t s_buffer[TEST_FIFO_BUFFER_LEN];

void setup_test(void) {
  fifo_init(&s_fifo, s_buffer, SIZEOF_ARRAY(s_buffer), sizeof(s_buffer[0]));
}

void teardown_test(void) {

}

void test_fifo_basic(void) {
  for (int i = TEST_FIFO_OFFSET; i < TEST_FIFO_BUFFER_LEN + TEST_FIFO_OFFSET; i++) {
    TEST_ASSERT_OK(fifo_push(&s_fifo, &i, sizeof(i)));
  }
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));

  uint32_t temp = 0;
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED,
                    fifo_push(&s_fifo, &temp, sizeof(temp)));

  TEST_ASSERT_OK(fifo_pop(&s_fifo, &temp, sizeof(temp)));
  TEST_ASSERT_EQUAL(TEST_FIFO_OFFSET, temp);
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN - 1, fifo_size(&s_fifo));

  temp = 0x54321;
  TEST_ASSERT_OK(fifo_push(&s_fifo, &temp, sizeof(temp)));

  while (fifo_size(&s_fifo) > 0) {
    uint32_t x = 0;
    TEST_ASSERT_OK(fifo_pop(&s_fifo, &x, sizeof(x)));
    if (fifo_size(&s_fifo) == 0) {
      TEST_ASSERT_EQUAL(temp, x);
    }
  }
}

void test_fifo_arr(void) {
  uint32_t send_arr[4] = { 0x12, 0x34, 0x56, 0x78 };
  uint32_t rx_arr[4] = { 0 };

  TEST_ASSERT_OK(fifo_push_arr(&s_fifo, send_arr, SIZEOF_ARRAY(send_arr), sizeof(send_arr[0])));

  TEST_ASSERT_OK(fifo_pop_arr(&s_fifo, rx_arr, SIZEOF_ARRAY(rx_arr), sizeof(rx_arr[0])));

  for (int i = 0; i < SIZEOF_ARRAY(send_arr); i++) {
    TEST_ASSERT_EQUAL(send_arr[i], rx_arr[i]);
  }

  for (int i = 0; i < TEST_FIFO_BUFFER_LEN - 2; i++) {
    uint32_t x = 0xDEAD;
    TEST_ASSERT_OK(fifo_push(&s_fifo, &x, sizeof(x)));
  }

  TEST_ASSERT_OK(fifo_pop(&s_fifo, NULL, 0));
  TEST_ASSERT_OK(fifo_pop(&s_fifo, NULL, 0));

  TEST_ASSERT_OK(fifo_push_arr(&s_fifo, send_arr, SIZEOF_ARRAY(send_arr), sizeof(send_arr[0])));
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));
}
