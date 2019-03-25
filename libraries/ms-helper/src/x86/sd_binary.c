#include "sd_binary.h"
#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"

StatusCode sd_card_init(SpiPort spi) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode sd_wait_data(SpiPort spi, uint8_t data) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode sd_read_blocks(SpiPort spi, uint32_t *pData, uint32_t ReadAddr,
                          uint32_t NumberOfBlocks) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode sd_write_blocks(SpiPort spi, uint32_t *pData, uint32_t WriteAddr,
                           uint32_t NumberOfBlocks) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode sd_multi_write_blocks(SpiPort spi, uint32_t *pData, uint32_t WriteAddr,
                                 uint32_t NumberOfBlocks) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode sd_is_initialized(SpiPort spi) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
