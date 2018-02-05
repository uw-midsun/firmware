#pragma once
#include <stdint.h>
#include <stddef.h>

#include "status.h"
#include "flash_mcu.h"
// Flash memory API - note that this is a raw API.
// Writes must be aligned
// note that write can only flip 1 -> 0
// erase flips entire page to 1
// each page is 2 kB (stm32f072) - 05x = 1kB
// each sector is 2 pages - always 4k total

// main flash memory can be programmed 16 bits at a time, but requires PG bit set to do so
// for ease of use, we'll just require u32s
// flash must be erased - can be done page by page

#define FLASH_ADDR_TO_PAGE(addr) (((uintptr_t)(addr) - (uintptr_t)FLASH_BASE_ADDR) / FLASH_PAGE_BYTES)
#define FLASH_PAGE_TO_ADDR(page) ((uintptr_t)(page) * FLASH_PAGE_BYTES + (uintptr_t)FLASH_BASE_ADDR)
#define FLASH_WRITE_BYTES FLASH_MCU_WRITE_BYTES
#define FLASH_PAGE_BYTES FLASH_MCU_PAGE_BYTES

StatusCode flash_init(void);

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len);

// buffer_len must be a multiple of FLASH_WRITE_BYTES
StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

StatusCode flash_erase(FlashPage page);
