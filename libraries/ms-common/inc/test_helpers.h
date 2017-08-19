#pragma once
// Test helper functions. These should only ever be called within a file in the test folder.

#include <stdint.h>

#include "status.h"
#include "unity.h"

// General use:
#define TEST_ASSERT_OK(code) TEST_ASSERT_EQUAL(STATUS_CODE_OK, (code))
#define TEST_ASSERT_NOT_OK(code) TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, (code))

// For testing soft_timer.h:
// TEST ONLY FUNCTION TO SET TIMER COUNTER FOR SOFT TIMERS UNSAFE TO CALL OUTSIDE A TEST.
void _test_soft_timer_set_counter(uint32_t counter_value);
