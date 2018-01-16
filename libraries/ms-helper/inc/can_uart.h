#pragma once
#include "uart.h"
#include "can_hw.h"

// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized

// Defined protocol: Header (u32), ID (u32), Data (u64), End (u8)
// Bits  | Field
// ------|--------------------
// 0:23  | marker (CTX || CRX)
// 24    | extended
// 25    | rtr
// 26    | err
// 27    | parity
// 28:31 | dlc

typedef struct CanUartPacket {
  union {
    struct {
      uint32_t start:24;
      uint32_t extended:1;
      uint32_t rtr:1;
      uint32_t reserved:1;
      uint32_t parity:1;
      uint32_t dlc:4;
    };
    uint32_t raw;
  } header;
  uint32_t id;
  uint64_t data;
  uint8_t end;
} CanUartPacket;

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
