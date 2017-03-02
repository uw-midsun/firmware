// HAL SPI

#pragma once

#include <stdint.h>

#include "gpio.h"
#include "status.h"

// For setting clock polarity
typedef enum {
  SPI_CPOLARITY_LOW = 0,
  SPI_CPOLARITY_HIGH,
} SPICpol;

// For setting clock phase
typedef enum {
  SPI_CPHASE_1EDGE = 0,
  SPI_CPHASE_2EDGE,
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
  SPICPolarity polarity;
  SPICPhase phase;
  SPIBaudRate baud_rate;
  SPIFirstBit first_bit;

  // GPIO pin settings
  GPIOAddress mosi_pin;
  GPIOAddress miso_pin;
  GPIOAddress sck_pin;
  GPIOAddress nss_pin;
}

// Note: spi_x = 0 for SPI1, spi_x = 1 for SPI2.

// Initializes a SPI peripheral.
  // Communication direction will always be set to full duplex.
  // Mode will always be set to master.
  // NSS is always set to be managed in software
  // Data size is always set to 8 bits
StatusCode spi_init(uint8_t spi_x, SPISettings *settings);

// Transmit 8 bits
StatusCode spi_send(uint8_t spi_x, uint8_t data);

// Transmits all messages in data
StatusCode spi_send_array(uint8_t spi_x, uint8_t *data, size_t data_length);

// Receive 8 bits
uint8_t spi_receive(uint8_t spi_x);

// Sends and receives 8 bits
uint8_t spi_exchange(uint8_t spi_x, uint8_t data);

// Sets or resets NSS pin
StatusCode spi_configure_NSS(uint8_t spi_x, SPISettings *settings, GPIOState state);

// Toggles NSS pin
StatusCode spi_toggle_NSS(uint8_t spi_x, SPISettings *settings);
