#pragma once
// CRC32 module
// Polynomial 0x04C11DB7, initial value 0xFFFFFFFF - standard CRC32 model
#include <stddef.h>
#include <stdint.h>
#include "status.h"

StatusCode crc32_init(void);

uint32_t crc32_arr(const uint8_t *buffer, size_t buffer_len);
