#pragma once

// Generic CAN HW <-> UART protocol
// Requires CAN HW, UART to be initialized

// Defined protocol: first u32
// Bits  | Field
// ------|--------------------
// 0:23  | marker (CTX || CRX)
// 24    | extended
// 25    | rtr
// 26    | err
// 27    | parity
// 28:31 | dlc

typedef struct CanUart {
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
} CanUart;
