#pragma once
#include <stdint.h>
#include "gpio.h"
#include "status.h"

typedef void (*UARTRxHandler)(const char *rx_str, size_t len, void *context);

typedef struct {
  uint32_t baudrate;
  UARTRxHandler rx_handler;
  void *context;

  GPIOAddress tx;
  GPIOAddress rx;
  GPIOAltFn alt_fn;
} UARTSettings;

// Assumes standard 8 N 1
StatusCode uart_init(UARTPort uart, UARTSettings *settings);

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len);
