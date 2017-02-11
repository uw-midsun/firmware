// HAL SPI

#pragma once

#include "gpio.h"
#include "interrupt.h"
#include "status.h"


typdef void (*spi_callback)(uint8_t spi_x, SPIITSource source, void *context);

// For setting data size
typedef enum {
  SPI_DATASIZE_4B = 0,
  SPI_DATASIZE_5B,
  SPI_DATASIZE_6B,
  SPI_DATASIZE_7B,
  SPI_DATASIZE_8B,
  SPI_DATASIZE_9B,
  SPI_DATASIZE_10B,
  SPI_DATASIZE_11B,
  SPI_DATASIZE_12B,
  SPI_DATASIZE_13B,
  SPI_DATASIZE_14B,
  SPI_DATASIZE_15B,
  SPI_DATASIZE_16B,
} SPIDataSize;

// For setting clock polarity
typedef enum {
  SPI_CPOLARITY_LOW = 0,
  SPI_CPOLARITY_HIGH,
} SPICPolarity;

// For setting clock phase
typedef enum {
  SPI_CPHASE_1EDGE = 0,
  SPI_CPHASE_2EDGE,
} SPICPhase;

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
  SPIDataSize data_size;
  SPICPolarity polarity;
  SPICPhase phase;
  SPIBaudRate baud_rate;

  // GPIO pin settings
  GPIOAddress mosi_pin;
  GPIOAddress miso_pin;
  GPIOAddress sck_pin;
  GPIOAddress nss_pin;
} 

// For selecting an interrupt source when registering a callback
typedef enum {
  SPI_IT_SOURCE_TXE = 0,    // Tx buffer empty
  SPI_IT_SOURCE_RXNE,       // Rx buffer not empty
  SPI_IT_SOURCE_ERR,        // error
} SPIITSource;

// Initializes a SPI peripheral.
  // Communication direction will always be set to full duplex.
  // Mode will always be set to master. 
  // NSS is always set to be managed in software
StatusCode spi_init(uint8_t spi_x, SPISettings *settings);

// Sets NSS high or low
StatusCode spi_set_NSS(uint8_t spi_x, SPISettings *settings, GPIOState state);

// Transmits all messages in data
StatusCode spi_send_data(uint8_t spi_x, uint8_t *data, size_t data_length);

// Registers a callback on a SPI peripheral on a given source.
StatusCode spi_register_interrupt(uint8_t spi_x, SPIITSource source, InterruptSettings *settings,
                                  spi_callback, void *context);
