// HAL SPI

// Interface for SPI; used to send and receive messages according to the SPI specification.

// To use, fill a SPISettings struct with the desired settings and call spi_init once.
// You cannot change these settings later.
// After initializing a SPI peripheral, you can use the spi_exchange and spi_set_cs_state
// functions. You must initialize each peripheral individually.

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

typedef enum {
  SPI_PERIPH_1 = 0,
  SPI_PERIPH_2,
} SPIPeriph;

// For setting clock polarity
typedef enum {
  SPI_CPOL_LOW = 0,
  SPI_CPOL_HIGH,
} SPICpol;

// For setting clock phase
typedef enum {
  SPI_CPHA_1EDGE = 0,
  SPI_CPHA_2EDGE,
} SPICpha;

// For setting baud rate prescaler
typedef enum {
  SPI_BAUDRATE_2 = 0,
  SPI_BAUDRATE_4,
  SPI_BAUDRATE_8,
  SPI_BAUDRATE_16,
  SPI_BAUDRATE_32,
  SPI_BAUDRATE_64,
  SPI_BAUDRATE_128,
  SPI_BAUDRATE_256,
} SPIBaudRate;

// For setting which bit is shifted out first
typedef enum {
  SPI_FIRSTBIT_MSB = 0,
  SPI_FIRSTBIT_LSB,
} SPIFirstBit;

// SPI init struct
typedef struct SPISettings {
  // SPI Settings
  SPICpol polarity;
  SPICpha phase;
  SPIBaudRate baud_rate;
  SPIFirstBit first_bit;

  // GPIO pin settings
  GPIOAddress mosi_pin;
  GPIOAddress miso_pin;
  GPIOAddress sck_pin;
  GPIOAddress cs_pin;
} SPISettings;

// Initialize a SPI peripheral.
StatusCode spi_init(SPIPeriph spi_x, SPISettings *settings);

// Sends all messages in tx_data and receives messages until rx_data is filled.
StatusCode spi_exchange(SPIPeriph spi_x, uint8_t *tx_data, size_t tx_len,
  uint8_t *rx_data, size_t rx_len);

// Sets CS high or low
StatusCode spi_set_cs_state(SPIPeriph spi_x, GPIOState state);
