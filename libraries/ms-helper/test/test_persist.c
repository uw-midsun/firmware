#include <inttypes.h>
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct TestPersistData {
  uint32_t foo;
  void *bar;
} TestPersistData;

static PersistStorage s_persist;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  flash_init();
  flash_erase(PERSIST_FLASH_PAGE);
}

void teardown_test(void) {
  flash_erase(PERSIST_FLASH_PAGE);
}

void test_persist_new(void) {
  TestPersistData data = { .foo = 0x12345678, .bar = &s_persist };

  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);
}

void test_persist_load_existing(void) {
  TestPersistData data = { .foo = 0x12345678, .bar = &s_persist };

  LOG_DEBUG("Creating initial persist\n");
  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  // Force some extra commits to simulate multiple changes
  for (uint32_t i = 0; i < 4; i++) {
    data.foo += i;
    LOG_DEBUG("Forcing commit %" PRIu32 " to persist layer\n", i);
    ret = persist_commit(&s_persist);
    TEST_ASSERT_OK(ret);
  }

  LOG_DEBUG("Setting up new persist layer\n");
  TestPersistData new_data = { 0 };
  ret = persist_init(&s_persist, &new_data, sizeof(new_data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(data.foo, new_data.foo);
  TEST_ASSERT_EQUAL(data.bar, new_data.bar);

  LOG_DEBUG("Committing new data\n");
  // Make sure we can commit data after reading a valid section
  new_data.foo = 0x87654321;
  new_data.bar = &new_data;
  ret = persist_commit(&s_persist);
  TEST_ASSERT_OK(ret);

  // Readback
  LOG_DEBUG("Reading back changed data\n");
  ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(new_data.foo, data.foo);
  TEST_ASSERT_EQUAL(new_data.bar, data.bar);
}

void test_persist_full_page(void) {
  uint32_t data[64] = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i;
  }

  LOG_DEBUG("Initializing persist layer\n");
  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  // Should be able to persist more than the maximum number of sections
  for (size_t i = 0; i < FLASH_PAGE_BYTES / sizeof(data) + 1; i++) {
    LOG_DEBUG("Attempting commit %" PRIu32 "\n", (uint32_t)i);
    data[i] += i;
    ret = persist_commit(&s_persist);
    TEST_ASSERT_OK(ret);
  }

  // Should load post-erase properly
  LOG_DEBUG("Loading persist data\n");
  uint32_t new_data[SIZEOF_ARRAY(data)] = { 0 };
  ret = persist_init(&s_persist, &new_data, sizeof(new_data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL_HEX32_ARRAY(data, new_data, SIZEOF_ARRAY(data));
}

void test_persist_size_change(void) {
  uint32_t data[4] = { 0x1, 0x2, 0x3, 0x4 };
  LOG_DEBUG("Initializing persist layer with size %" PRIu32 "\n", (uint32_t)sizeof(data));
  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  uint32_t small_data[2] = { 0x4, 0x5 };
  LOG_DEBUG("Initializing persist layer with size %" PRIu32 "\n", (uint32_t)sizeof(small_data));
  ret = persist_init(&s_persist, &small_data, sizeof(small_data));
  TEST_ASSERT_OK(ret);

  // Make sure the blob data was not overwritten
  TEST_ASSERT_EQUAL(0x4, small_data[0]);
  TEST_ASSERT_EQUAL(0x5, small_data[1]);

  // Switch back to larger data size
  LOG_DEBUG("Initializing persist layer with size %" PRIu32 "\n", (uint32_t)sizeof(data));
  ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(data[0], 0x1);
  TEST_ASSERT_EQUAL(data[1], 0x2);
  TEST_ASSERT_EQUAL(data[2], 0x3);
  TEST_ASSERT_EQUAL(data[3], 0x4);
}

void test_persist_change_periodic(void) {
  uint32_t data[4] = { 0 };
  LOG_DEBUG("Initializing persist layer with 0s\n");
  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  // Change the blob data
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i;
  }

  LOG_DEBUG("Data changed - delaying 2 periods (should only see 1 commit)\n");
  // Delay with some leeway - should only see 1 commit
  delay_ms(PERSIST_COMMIT_TIMEOUT_MS * 2 + 10);

  // Reload the persist layer
  LOG_DEBUG("Reloading persist layer\n");
  uint32_t readback[SIZEOF_ARRAY(data)] = { 0 };
  ret = persist_init(&s_persist, &readback, sizeof(readback));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL_HEX32_ARRAY(data, readback, SIZEOF_ARRAY(data));
}
