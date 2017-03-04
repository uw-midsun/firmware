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

// For setting interrupt source
typedef enum {
  USART_IT_SOURCE_TXE = 0,
  USART_IT_SOURCE_RXNE,
  USART_IT_SOURCE_TC,
  USART_IT_SOURCE_IDLE,
  USART_IT_SOURCE_CTS,
  USART_IT_SOURCE_LBD,
  USART_IT_SOURCE_PE,
  USART_IT_SOURCE_ERR,
} USARTITSource;

// USART settings
typedef struct {
  uint32_t baud_rate;
  USARTStopBits stop_bits;
  USARTParity parity;
  USARTMode mode;
  GPIOAddress tx_address;
  GPIOAddress rx_address;
} USARTSettings;


typedef void (*usart_it_callback)(uint8_t periph, void *context);

// Registers new callback
StatusCode usart_register_interrupt(uint8_t address,
  InterruptSettings *settings, InterruptEdge edge,
  usart_it_callback callback, void *context);

// Triggers interrupt
StatusCode usart_it_trigger_interrupt(uint8_t address);

// Initializes USART with settings
StatusCode usart_init(uint8_t periph, USARTSettings *settings);

// Configures interrupt source
StatusCode usart_it_config(uint8_t periph, USARTITSource source);

// Transmits data
StatusCode usart_send_data(USARTSettings *settings, uint16_t data);

// Receives data
StatusCode usart_receive(USARTSettings *settings);
