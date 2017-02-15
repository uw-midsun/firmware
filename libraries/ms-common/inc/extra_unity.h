#pragma once

#include <stdint.h>

#include "status.h"
#include "unity.h"

#define TEST_ASSERT_OK(code) TEST_ASSERT_EQUAL(STATUS_CODE_OK, (code))

// TEST ONLY FUNCTION TO SET TIMER COUNTER FOR SOFT TIMERS UNSAFE TO CALL OUTSIDE A TEST.
void TEST_SOFT_TIMER_SET_COUNTER(uint32_t counter_value);