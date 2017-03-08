// HAL SPI

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

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
StatusCode spi_init(uint8_t spi_x, SPISettings *settings);

// Sends all messages in tbuf and receives messages until rbuf is filled.
StatusCode spi_exchange(uint8_t spi_x, uint8_t *tbuf, uint8_t *rbuf,
  size_t t_length, size_t r_length);

// Sets CS high or low
StatusCode spi_set_cs_state(uint8_t spi_x, GPIOState state);
