#pragma once
// Generic blocking SPI driver
// Requires GPIO to be initialized.
#include <stddef.h>
#include "gpio.h"
#include "spi_mcu.h"
#include "status.h"

typedef enum {
  SPI_MODE_0 = 0,  // CPOL: 0 CPHA: 0
  SPI_MODE_1,      // CPOL: 0 CPHA: 1
  SPI_MODE_2,      // CPOL: 1 CPHA: 0
  SPI_MODE_3,      // CPOL: 1 CPHA: 1
  NUM_SPI_MODES,
} SpiMode;

typedef struct {
  uint32_t baudrate;
  SpiMode mode;

  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} SpiSettings;

// Note that our prescalers on STM32 must be a power of 2, so the actual baudrate may not be
// exactly as requested. Please verify that the actual baudrate is within bounds.
StatusCode spi_init(SpiPort spi, const SpiSettings *settings);

StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len);
