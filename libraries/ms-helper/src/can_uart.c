#include "can_uart.h"

// CTX
#define CAN_UART_TX_MARKER 0x435458
// CRX
#define CAN_UART_RX_MARKER 0x435258

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
      uint32_t marker:24;
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
  uint8_t newline;
} CanUartPacket;

static void prv_tx_uart()

static void prv_rx_uart(const uint8_t *rx_arr, size_t len, void *context) {
  if (len < 8) {
    // no way this is valid
    return;
  }

  CanUartPacket packet;
  packet.header = *(uint32_t *)rx_arr;
  if (packet.header.marker != CAN_UART_RX_MARKER && packet.header.start != CAN_UART_TX_MARKER) {
    // invalid marker
    return;
  }
  packet.id = *(uint32_t *)(rx_arr + 4);
  if (dlc > 8) {
    // dlc too large
    return;
  }
}

static void prv_handle_can_rx(void *context) {
  CanUart *can_uart = context;

  uint32_t id;
  bool extended;
  uint64_t data;
  size_t len;
  while (can_hw_receive(&id, &extended, &data, &len)) {
    if (can_uart->rx_cb != NULL) {
      can_uart->rx_cb(can_uart, id, extended, data, len, can_uart->context);
    }
  }
}

void can_uart_init(CanUart *can_uart) {
  if (can_uart->hook_can_hw) {
    // slave
    can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_handle_can_rx, can_uart);
  }

  // uart set handler
}

void can_uart_req_tx(const CanUart *can_uart, uint32_t id, bool extended,
                     const uint8_t *data, size_t len) {

}
