#pragma once
// Non-blocking UART driver
// Requires GPIO and interrupts to be initialized.

// Uses internal FIFOs to buffer RX and TX.
#include <stdint.h>
#include "fifo.h"
#include "gpio.h"
#include "status.h"
#include "uart_mcu.h"

#define UART_MAX_BUFFER_LEN 1024

typedef void (*UARTRxHandler)(const uint8_t *rx_arr, size_t len, void *context);

typedef struct {
  UARTRxHandler rx_handler;
  void *context;

  volatile Fifo tx_fifo;
  volatile uint8_t tx_buf[UART_MAX_BUFFER_LEN];
  volatile Fifo rx_fifo;
  volatile uint8_t rx_buf[UART_MAX_BUFFER_LEN];

  uint8_t rx_line_buf[UART_MAX_BUFFER_LEN + 1];
  char delimiter;
} UARTStorage;

typedef struct {
  uint32_t baudrate;
  UARTRxHandler rx_handler;
  void *context;

  GPIOAddress tx;
  GPIOAddress rx;
  GPIOAltFn alt_fn;
} UARTSettings;

// Assumes standard 8 N 1
// Registers a handler to be called when a newline is encountered or the buffer is full.
// Storage should be persistent through the program.
StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage);

// Overrides any currently set handler
StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context);

// Sets the delimiter used to break up lines between callbacks
// Note that the default delimiter is \n
StatusCode uart_set_delimiter(UARTPort uart, char delimiter);

// Non-blocking TX
StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len);
