#pragma once
// UART Interface
#include <stdint.h>

#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "stm32f0xx_usart.h"

// For setting word length
typedef enum {
  UART_WORD_LENGTH_7 = 0,
  UART_WORD_LENGTH_8,
  UART_WORD_LENGTH_9
} UARTWordLength;

// For setting number of stop bits
typedef enum {
  UART_STOP_BITS_1 = 0,
  UART_STOP_BITS_2,
} UARTStopBits;

// For setting parity mode
typedef enum {
  UART_PARITY_DISABLE = 0,
  UART_PARITY_EVEN,
  UART_PARITY_ODD,
} UARTParity;

// For setting receive or transmit mode
typedef enum {
  UART_MODE_RXTX = 0,
  UART_MODE_RX,
  UART_MODE_TX,
} UARTMode;

// UART settings
typedef struct {
  uint32_t baud_rate;
  UARTWordLength word_length;
  UARTStopBits stop_bits;
  UARTParity parity;
  UARTMode mode;
  GPIOAddress tx_address;
  GPIOAddress rx_address;
} UARTSettings;

// Initializes UART with settings
StatusCode uart_init(UARTSettings *settings, USART_TypeDef *uart, USART_InitTypeDef* uart_init);

// Transmits data
StatusCode uart_send(USART_TypeDef *uart, uint8_t data);

// Tranmits array of data
StatusCode uart_send_array(USART_TypeDef *uart, uint8_t *data, uint8_t length);

// Receives data
uint8_t uart_receive(USART_TypeDef *uart);
