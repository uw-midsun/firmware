#pragma once
#include "uart.h"
#include "can_hw.h"

// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized

typedef struct CanUart {
  UARTPort uart;
  CanUartRxCb rx_cb;
  void *context;
  bool hook_can_hw; // whether the module should hook directly into the CAN HW module
} CanUart;

typedef void (*CanUartRxCb)(const CanUart *can_uart, uint32_t id, bool extended,
                            const uint8_t *data, size_t len, void *context);

void can_uart_init(CanUart *can_uart);

// Intended to request a TX on the receiver
// i.e. from Master to Slave
void can_uart_req_tx(const CanUart *can_uart, uint32_t id, bool extended,
                     const uint8_t *data, size_t len);
