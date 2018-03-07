#include "generic_can_uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can_uart.h"
#include "generic_can.h"
#include "status.h"

static CanUart s_can_uart;
static GenericCanInterface s_interface;

// CanUartRxCb
static void prv_generic_can_uart_rx_handler(const CanUart *can_uart, uint32_t id, bool extended,
                                            const uint64_t *data, size_t dlc, void *context) {
  (void)can_uart;
  GenericCanUart *gcu = context;
  for (size_t i = 0; i < NUM_GENERIC_CAN_UART_RX_HANDLERS; i++) {
    if (id == gcu->rx_storage[i].id && gcu->rx_storage[i].rx_handler != NULL &&
        gcu->rx_storage[i].enabled) {
      const GenericCanMsg msg = {
        .id = id,
        .extended = extended,
        .data = *data,
        .dlc = dlc,
      };
      gcu->rx_storage[i].rx_handler(&msg, gcu->rx_storage[i].context);
      break;
    }
  }
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanUart *gcu = (GenericCanUart *)can;
  if (gcu->base.interface != &s_interface || gcu->can_uart != &s_can_uart) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanUart.");
  }

  return can_uart_req_slave_tx(gcu->can_uart, msg->id, msg->extended, &msg->data, msg->dlc);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                  void *context) {
  GenericCanUart *gcu = (GenericCanUart *)can;
  if (gcu->base.interface != &s_interface || gcu->can_uart != &s_can_uart) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanUart.");
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_UART_RX_HANDLERS; i++) {
    if (gcu->rx_storage[i].rx_handler == NULL && !gcu->rx_storage[i].enabled) {
      gcu->rx_storage[i].id = id;
      gcu->rx_storage[i].rx_handler = rx_handler;
      gcu->rx_storage[i].context = context;
      gcu->rx_storage[i].enabled = true;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}

static StatusCode prv_set_rx(GenericCan *can, uint32_t id, bool state) {
  GenericCanUart *gcu = (GenericCanUart *)can;
  if (gcu->base.interface != &s_interface || gcu->can_uart != &s_can_uart) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanUart.");
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_UART_RX_HANDLERS; i++) {
    if (gcu->rx_storage[i].id == id && gcu->rx_storage[i].rx_handler != NULL) {
      gcu->rx_storage[i].enabled = state;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_UNINITIALIZED);
}

// enable_rx
static StatusCode prv_enable_rx(GenericCan *can, uint32_t id) {
  return prv_set_rx(can, id, true);
}

// disable_rx
static StatusCode prv_disable_rx(GenericCan *can, uint32_t id) {
  return prv_set_rx(can, id, false);
}

StatusCode generic_can_uart_init(UARTPort port, GenericCanUart *out) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;
  s_interface.enable_rx = prv_enable_rx;
  s_interface.disable_rx = prv_disable_rx;

  s_can_uart.uart = port;
  s_can_uart.context = (void *)out;
  s_can_uart.rx_cb = prv_generic_can_uart_rx_handler;
  status_ok_or_return(can_uart_init(&s_can_uart));
  out->can_uart = &s_can_uart;

  memset(out->rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_UART_RX_HANDLERS);

  out->base.interface = &s_interface;
  return STATUS_CODE_OK;
}
