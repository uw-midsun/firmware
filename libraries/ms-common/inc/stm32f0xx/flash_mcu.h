#pragma once

extern uint32_t __flash_start;
extern uint32_t __flash_size;
extern uint32_t __flash_page_size;

#define FLASH_BASE_ADDR ((uintptr_t)&__flash_start)
#define FLASH_SIZE_BYTES ((size_t)&__flash_size)
#define FLASH_END_ADDR (FLASH_BASE_ADDR + FLASH_SIZE_BYTES)

#define FLASH_MCU_WRITE_BYTES 4
#define FLASH_MCU_PAGE_BYTES ((size_t)&__flash_page_size)

typedef enum {
  FLASH_PAGE_0 = 0,
  FLASH_PAGE_1,
  FLASH_PAGE_2,
  FLASH_PAGE_3,
  FLASH_PAGE_4,
  FLASH_PAGE_5,
  FLASH_PAGE_6, // TODO: fill this out? there are 128 pages
  NUM_FLASH_PAGES;
} FlashPage;
