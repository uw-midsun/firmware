#include "spi.h"

StatusCode spi_init(SPIPort spi, const SPISettings *settings) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode spi_exchange(SPIPort spi, uint8_t *tx_data, size_t tx_len,
                        uint8_t *rx_data, size_t rx_len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
