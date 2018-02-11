#include "unity.h"
#include "persist.h"
#include "test_helpers.h"
#include "log.h"

typedef struct TestPersistData {
  uint32_t foo;
  void *bar;
} TestPersistData;

static PersistStorage s_persist;

void setup_test(void) {
  flash_init();
  flash_erase(PERSIST_FLASH_PAGE);
}

void teardown_test(void) {
  flash_erase(PERSIST_FLASH_PAGE);
}

void test_persist_new(void) {
  TestPersistData data = {
    .foo = 0x12345678,
    .bar = &s_persist
  };

  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);
}

void test_persist_load_existing(void) {
  TestPersistData old_data = {
    .foo = 0x12345678,
    .bar = &s_persist
  };

  LOG_DEBUG("Creating initial persist\n");
  StatusCode ret = persist_init(&s_persist, &old_data, sizeof(old_data));
  TEST_ASSERT_OK(ret);

  LOG_DEBUG("Forcing initial commit to persist layer\n");
  ret = persist_commit(&s_persist);
  TEST_ASSERT_OK(ret);

  for (uint32_t i = 0; i < 4; i++) {
    old_data.foo += i;
    LOG_DEBUG("Forcing commit %d to persist layer\n", i);
    ret = persist_commit(&s_persist);
    TEST_ASSERT_OK(ret);
  }

  LOG_DEBUG("Setting up new persist layer\n");
  TestPersistData new_data = { 0 };
  ret = persist_init(&s_persist, &new_data, sizeof(new_data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(old_data.foo, new_data.foo);
  TEST_ASSERT_EQUAL(old_data.bar, new_data.bar);
}

void test_persist_full_page(void) {
  uint64_t data[32] = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i;
  }

  LOG_DEBUG("Initializing persist layer\n");
  StatusCode ret = persist_init(&s_persist, &data, sizeof(data));
  TEST_ASSERT_OK(ret);

  // Should be able to persist more than the maximum number of sections
  for (size_t i = 0; i < FLASH_PAGE_BYTES / sizeof(data) + 1; i++) {
    LOG_DEBUG("Attempting commit %ld\n", i);
    data[i] += i;
    ret = persist_commit(&s_persist);
    TEST_ASSERT_OK(ret);
  }

  // Should load post-erase properly
  LOG_DEBUG("Loading persist data\n");
  uint64_t new_data[SIZEOF_ARRAY(data)] = { 0 };
  ret = persist_init(&s_persist, &new_data, sizeof(new_data));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL_HEX64_ARRAY(data, new_data, SIZEOF_ARRAY(data));
}
