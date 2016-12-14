#pragma once
#include "status.h"
#include "unity.h"

#define TEST_ASSERT_OK(code) TEST_ASSERT_EQUAL(STATUS_CODE_OK, (code))
