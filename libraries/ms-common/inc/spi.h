// SPI HAL Interface

#include "interrupt.h"
#include "status.h"
#include "stm32f0xx.h"

typdef void (*spi_callback)(SPIPeriph spi_x, void *context);

// For selecting which peripheral
typedef enum {
  SPI_PERIPH_1 = 0,
  SPI_PERIPH_2,
} SPIPeriph;

// For setting communication directions
typedef enum {
  SPI_DIR_2LINES_FD = 0,
  SPI_DIR_2LINES_RX_ONLY,
  SPI_DIR_1LINE_RX,
  SPI_DIR_1LINE_TX,
} SPIDir;

// For setting master or slave mode
typedef enum {
  SPI_MODE_MASTER = 0,
  SPI_MODE_SLAVE,
} SPIMode;

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
  SPI_CPOL_LOW = 0,
  SPI_CPOL_HIGH,
} SPICPOL;

// For setting clock phase
typedef enum {
  SPI_CPHA_1EDGE = 0,
  SPI_CPHA_2EDGE,
} SPICPHA;

// For configuring NSS pin
typedef enum {
  SPI_NSS_SOFT = 0,
  SPI_NSS_HARD,
} SPINSS;

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
  SPIDir direction;
  SPIMode mode;
  SPIDataSize data_size;
  SPICPOL polarity;
  SPICPHA phase;
  SPINSS nss;
  SPIBaudRate baud_rate;
}

// For selecting an interrupt source when registering a callback
typedef enum {
  SPI_IT_SOURCE_TXE = 0,    // Tx buffer empty
  SPI_IT_SOURCE_RXNE,       // Rx buffer not empty
  SPI_IT_SOURCE_ERR,        // error
} SPIITSource;

// Initializes a SPI peripheral.
StatusCode spi_init(SPIPeriph spi_x, SPISettings *settings);

// Transmits 8 bits of data through a SPI peripheral.
StatusCode spi_send_data8(SPIPeriph spi_x, uint8_t data);

// Transmits 16 bits of data through a SPI peripheral.
StatusCode spi_send_data16(SPIPeriph spi_x, uint16_t data);

// Registers a callback on a SPI peripheral on a given source.
StatusCode spi_register_interrupt(SPIPeriph spi_x, SPIITSource source, InterruptSettings *settings,
                                  spi_callback, void *context);
