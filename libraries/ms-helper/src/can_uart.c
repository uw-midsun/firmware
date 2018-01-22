#include "can_uart.h"
#include <string.h>
#include "log.h"

// CTX
#define CAN_UART_TX_MARKER 0x585443
// CRX
#define CAN_UART_RX_MARKER 0x585243

// Defined protocol: Header (u32), ID (u32), Data (u64), newline (u8)
// Bits  | Field
// ------|--------------------
// 0:23  | marker (CTX || CRX)
// 24    | extended
// 25    | rtr
// 26-27 | reserved
// 28:31 | dlc

#define CAN_UART_BUILD_HEADER(marker, extended, rtr, dlc)             \
  (((uint32_t)(marker)&0xFFFFFF) | ((uint32_t)(extended)&0x1) << 24 | \
   ((uint32_t)(rtr)&0x1) << 25 | ((uint32_t)(dlc)&0xF) << 28)
#define CAN_UART_EXTRACT_HEADER(header, marker, extended, rtr, dlc) \
  ({                                                                \
    uint32_t _header = header;                                      \
    *(marker) = _header & 0xFFFFFF;                                 \
    *(extended) = (_header >> 25) & 0x1;                            \
    *(rtr) = (_header >> 25) & 0x1;                                 \
    *(dlc) = (_header >> 28) & 0xF;                                 \
    true;                                                           \
  })

typedef struct CanUartPacket {
  uint32_t header;
  uint32_t id;
  uint64_t data;
} CanUartPacket;

static void prv_rx_uart(const uint8_t *rx_arr, size_t len, void *context) {
  CanUart *can_uart = context;

  CanUartPacket packet;
  if (len < sizeof(packet)) {
    // no way this is valid
    return;
  }
  memcpy(&packet, rx_arr, sizeof(packet));

  uint32_t marker;
  size_t dlc;
  bool extended, rtr;
  CAN_UART_EXTRACT_HEADER(packet.header, &marker, &extended, &rtr, &dlc);

  if (marker == CAN_UART_RX_MARKER) {
    // RX'd CAN message - alert system
    if (can_uart->rx_cb != NULL) {
      can_uart->rx_cb(can_uart, packet.id, extended, &packet.data, dlc, can_uart->context);
    }
  } else if (marker == CAN_UART_TX_MARKER) {
    // TX request - attempt to transmit message
    can_hw_transmit(packet.id, extended, (uint8_t *)&packet.data, dlc);
  }
}

static void prv_handle_can_rx(void *context) {
  CanUart *can_uart = context;

  uint32_t id;
  bool extended;
  uint64_t data;
  size_t dlc;
  while (can_hw_receive(&id, &extended, &data, &dlc)) {
    CanUartPacket packet = { .header =
                                 CAN_UART_BUILD_HEADER(CAN_UART_RX_MARKER, extended, false, dlc),
                             .id = id,
                             .data = data };
    uint8_t newline = '\n';

    StatusCode ret = uart_tx(can_uart->uart, (uint8_t *)&packet, sizeof(packet));
    if (ret == STATUS_CODE_OK) {
      uart_tx(can_uart->uart, &newline, sizeof(newline));
    }
  }
}

StatusCode can_uart_init(CanUart *can_uart) {
  return uart_set_rx_handler(can_uart->uart, prv_rx_uart, can_uart);
}

StatusCode can_uart_hook_can_hw(CanUart *can_uart) {
  return can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_handle_can_rx, can_uart);
}

StatusCode can_uart_req_tx(const CanUart *can_uart, uint32_t id, bool extended,
                           const uint64_t *data, size_t dlc) {
  CanUartPacket packet = { .header =
                               CAN_UART_BUILD_HEADER(CAN_UART_TX_MARKER, extended, false, dlc),
                           .id = id,
                           .data = *data };
  uint8_t newline = '\n';

  StatusCode ret = uart_tx(can_uart->uart, (uint8_t *)&packet, sizeof(packet));
  status_ok_or_return(ret);

  // Add trailing newline
  return uart_tx(can_uart->uart, &newline, sizeof(newline));
}
