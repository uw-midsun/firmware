#pragma once
// Generic blocking SPI driver
// Requires GPIO to be initialized.
#include <stddef.h>
#include "spi_mcu.h"
#include "gpio.h"
#include "status.h"

typedef enum {
  SPI_MODE_0 = 0, // CPOL: 0 CPHA: 0
  SPI_MODE_1,     // CPOL: 0 CPHA: 1
  SPI_MODE_2,     // CPOL: 1 CPHA: 0
  SPI_MODE_3      // CPOL: 1 CPHA: 1
} SPIMode;

typedef struct {
  uint32_t baudrate;
  SPIMode mode;

  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;
  GPIOAddress cs;
} SPISettings;

StatusCode spi_init(SPIPort spi, SPISettings *settings);

StatusCode spi_exchange(SPIPort spi, uint8_t *tx_data, size_t tx_len,
                        uint8_t *rx_data, size_t rx_len);
