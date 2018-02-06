#include "flash.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define FLASH_FILENAME "x86_flash"

static FILE *s_flash_fp = NULL;

StatusCode flash_init(void) {
  if (s_flash_fp != NULL) {
    fclose(s_flash_fp);
  }

  s_flash_fp = fopen(FLASH_FILENAME, "r+b");
  if (s_flash_fp == NULL) {
    LOG_DEBUG("Setting up new flash file\n");
    s_flash_fp = fopen(FLASH_FILENAME, "w+b");
    for (int i = 0; i < NUM_FLASH_PAGES; i++) {
      flash_erase(i);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len) {
  if (buffer_len < read_bytes || address < FLASH_BASE_ADDR ||
      (address + read_bytes) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  fseek(s_flash_fp, (long int)address, SEEK_SET);
  size_t ret = fread(buffer, 1, read_bytes, s_flash_fp);
  (void)ret;

  return STATUS_CODE_OK;
}

// buffer_len must be a multiple of FLASH_WRITE_BYTES
StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len) {
  if (address < FLASH_BASE_ADDR || (address + buffer_len) > FLASH_END_ADDR) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  } else if (buffer_len % FLASH_WRITE_BYTES != 0 || address % FLASH_WRITE_BYTES != 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t read_buffer[buffer_len];

  fseek(s_flash_fp, (long int)address, SEEK_SET);
  size_t read = fread(read_buffer, 1, buffer_len, s_flash_fp);
  (void)read;

  for (size_t i = 0; i < buffer_len; i++) {
    if (read_buffer[i] != 0xFF) {
      return status_msg(STATUS_CODE_INTERNAL_ERROR, "Flash: Attempted to write to already written flash");
    }
  }

  fseek(s_flash_fp, (long int)address, SEEK_SET);
  fwrite(buffer, 1, buffer_len, s_flash_fp);
  fflush(s_flash_fp);

  return STATUS_CODE_OK;
}

StatusCode flash_erase(FlashPage page) {
  if (page > NUM_FLASH_PAGES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  char buffer[FLASH_PAGE_BYTES];
  memset(buffer, 0xFF, sizeof(buffer));

  fseek(s_flash_fp, (long int)FLASH_PAGE_TO_ADDR(page), SEEK_SET);
  fwrite(buffer, 1, sizeof(buffer), s_flash_fp);
  fflush(s_flash_fp);

  return STATUS_CODE_OK;
}
