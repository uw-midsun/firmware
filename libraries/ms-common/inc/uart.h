#pragma once
#include <stdint.h>
#include "gpio.h"
#include "status.h"
#include "fifo.h"

#define UART_MAX_BUFFER_LEN 512

// TODO: Replace, just for now
typedef uint8_t UARTPort;

typedef void (*UARTRxHandler)(const char *rx_str, size_t len, void *context);

typedef struct {
  UARTRxHandler rx_handler;
  void *context;

  volatile Fifo tx_fifo;
  volatile uint8_t tx_buf[UART_MAX_BUFFER_LEN];
  volatile Fifo rx_fifo;
  volatile uint8_t rx_buf[UART_MAX_BUFFER_LEN];
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
// Storage should be persistent through the program
StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage);

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len);
