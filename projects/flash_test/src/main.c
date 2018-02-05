#include <stdint.h>
#include <string.h>
#include "misc.h"
#include "flash.h"
#include "log.h"

int main(void) {
  flash_init();

  const char buffer[32] = "hello there\nthis is a test";
  char output[100];

  StatusCode ret = flash_erase(0);
  if (!status_ok(ret)) {
    LOG_DEBUG("flash erase failed\n");
  }

  LOG_DEBUG("Writing buffer of size %ld\n", SIZEOF_ARRAY(buffer));
  ret = flash_write(FLASH_BASE_ADDR, buffer, SIZEOF_ARRAY(buffer));
  if (!status_ok(ret)) {
    LOG_DEBUG("flash write failed\n");
  }

  ret = flash_read(FLASH_BASE_ADDR, SIZEOF_ARRAY(buffer), (uint8_t *)output, SIZEOF_ARRAY(output));
  if (!status_ok(ret)) {
    LOG_DEBUG("flash read failed\n");
  }

  LOG_DEBUG("%s\n", output);

  LOG_DEBUG("Writing same buffer of size %ld\n", SIZEOF_ARRAY(buffer));
  ret = flash_write(FLASH_BASE_ADDR, buffer, SIZEOF_ARRAY(buffer));
  if (!status_ok(ret)) {
    LOG_DEBUG("flash write 2 failed\n");
  }

  uint8_t data[] = { 'h' & ~0x40, 'e', 'l', 'l' };
  ret = flash_write(FLASH_BASE_ADDR, data, SIZEOF_ARRAY(data));
  if (!status_ok(ret)) {
    LOG_DEBUG("flash write 3 failed\n");
  }

  data[0] = ('h' & ~0x40) | 0x01;
  ret = flash_write(FLASH_BASE_ADDR, data, SIZEOF_ARRAY(data));
  if (!status_ok(ret)) {
    LOG_DEBUG("flash write 4 failed\n");
  }

  LOG_DEBUG("end\n");

  return 0;
}