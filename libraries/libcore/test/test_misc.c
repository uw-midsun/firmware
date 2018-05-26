#include <stdbool.h>
#include "log.h"
#include "misc.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

// Standard test
void test_misc_strtok_d_0(void) {
  char *test = "Hello World";
  char *new;
  TEST_ASSERT(strtok_d(test, ' ', &new, NULL) == false);
  TEST_ASSERT_EQUAL_STRING("World", new);
}

// Double delimiter
void test_misc_strtok_d_1(void) {
  char *test = "Hello,,World";
  char *new;
  TEST_ASSERT(strtok_d(test, ',', &new, NULL) == true);
  TEST_ASSERT_EQUAL_STRING("World", new);
}

// Double delimiter, and three sections after split
void test_misc_strtok_d_2(void) {
  char *test = "Hello,,World,Again";
  char *new;
  TEST_ASSERT(strtok_d(test, ',', &new, NULL) == true);
  TEST_ASSERT_EQUAL_STRING("World,Again", new);

  TEST_ASSERT(strtok_d(new, ',', &new, NULL) == false);
  TEST_ASSERT_EQUAL_STRING("Again", new);
}

// Null string
void test_misc_strtok_d_3(void) {
  char *test = "";
  char *new;
  TEST_ASSERT(strtok_d(test, ',', &new, NULL) == false);
  TEST_ASSERT(new == NULL);
}

// Double delimiter, and three sections after split
void test_misc_strtok_d_4(void) {
  char *test = "Hello,,World,Again";
  char *new = "unmodified string";
  bool do_nothing = true;
  TEST_ASSERT(strtok_d(test, ',', &new, &do_nothing) == false);
  TEST_ASSERT(!do_nothing);
  TEST_ASSERT_EQUAL_STRING("unmodified string", new);
}
