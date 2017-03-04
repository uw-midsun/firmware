#pragma once
// USART Interface
#include <stdint.h>

#include "gpio.h"
#include "interrupt.h"
#include "status.h"

// For setting number of stop bits
typedef enum {
  USART_STOP_BITS_1 = 0,
  USART_STOP_BITS_2,
} USARTStopBits;

// For setting parity mode
typedef enum {
  USART_PARITY_DISABLE = 0,
  USART_PARITY_EVEN,
  USART_PARITY_ODD,
} USARTParity;

// For setting receive or transmit mode
typedef enum {
  USART_MODE_RX = 0,
  USART_MODE_TX,
} USARTMode;

// USART settings
typedef struct {
  uint32_t baud_rate;
  USARTStopBits stop_bits;
  USARTParity parity;
  USARTMode mode;
  GPIOAddress tx_address;
  GPIOAddress rx_address;
} USARTSettings;

// Initializes USART with settings
StatusCode usart_init(uint8_t periph, USARTSettings *settings);

// Transmits data
StatusCode usart_transmit(USARTSettings *settings, uint16_t data);

// Tranmits array of data
StatusCode usart_transmit_array(USARTSettings *settings, uint8_t *data, uint8_t length);

// Receives data
StatusCode usart_receive(USARTSettings *settings);
