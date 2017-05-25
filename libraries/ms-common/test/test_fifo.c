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
    LOG_DEBUG("Pushing 0x%x\n", i);
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
    LOG_DEBUG("0x%x\n", x);
  }
}
