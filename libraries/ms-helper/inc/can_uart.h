#pragma once
#include "uart.h"
#include "can_hw.h"
#include "status.h"

// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized

// Process RX'd CAN message from UART
struct CanUart;
typedef void (*CanUartRxCb)(const struct CanUart *can_uart, uint32_t id, bool extended,
                            const uint64_t *data, size_t dlc, void *context);

typedef struct CanUart {
  UARTPort uart;
  CanUartRxCb rx_cb;
  void *context;
} CanUart;

// Note that the provided UART port will have its RX handler overwritten.
StatusCode can_uart_init(CanUart *can_uart);

// Overrides the CAN HW RX handler to pass RX'd CAN messages over UART
StatusCode can_uart_hook_can_hw(CanUart *can_uart);

// Intended to request a TX on the receiver
// i.e. from Master to Slave
StatusCode can_uart_req_tx(const CanUart *can_uart, uint32_t id, bool extended,
                           const uint64_t *data, size_t dlc);
