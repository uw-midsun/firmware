#include "flash.h"

StatusCode flash_init(void) {
  // Flash prefetch and latency are set up in system init
  return STATUS_CODE_OK;
}

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len) {
  if (buffer_len < read_bytes || address < FLASH_BASE_ADDR ||
      (address + read_bytes) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // may want to move all flash-related functions into RAM so we don't block the CPU
  memcpy(buffer, (void *)address, read_bytes);

  return STATUS_CODE_OK;
}

StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < FLASH_BASE_ADDR || (address + buffer_len) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  } else if (buffer_len % FLASH_WRITE_BYTES != 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t *data = buffer;
  size_t data_len = buffer_len / FLASH_WRITE_BYTES;

  FLASH_Unlock();
  for (size_t i = 0; i < data_len; i++) {
    FLASH_ProgramWord(address + (i * FLASH_WRITE_BYTES), data[i]);
  }
  FLASH_Lock();

  return STATUS_CODE_OK;
}

StatusCode flash_erase(FlashPage page) {
  if (page > NUM_FLASH_PAGES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  FLASH_Unlock();
  FLASH_ErasePage(FLASH_PAGE_TO_ADDR(page));
  FLASH_Lock();

  return STATUS_CODE_OK;
}
