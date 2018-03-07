#include "generic_can_can.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "status.h"

static GenericCanInterface s_interface;

// CanRxHandlerCb
static StatusCode prv_generic_can_can_rx_handler(const CANMessage *msg, void *context,
                                                 CANAckStatus *ack_reply) {
  (void)ack_reply;
  GenericCanCan *gcc = context;
  GenericCanMsg generic_msg = { 0 };
  // If we cannot turn the message into a GenericCanMsg then there is a problem with it.
  status_ok_or_return(can_message_to_generic_can_message(msg, &generic_msg));
  for (size_t i = 0; i < NUM_GENERIC_CAN_CAN_RX_HANDLERS; i++) {
    if (generic_msg.id == gcc->rx_storage[i].id && gcc->rx_storage[i].rx_handler != NULL &&
        gcc->rx_storage[i].enabled) {
      gcc->rx_storage[i].rx_handler(&generic_msg, gcc->rx_storage[i].context);
      break;
    }
  }

  // Assume that unhandled messages aren't of interest.
  return STATUS_CODE_OK;
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanCan *gcc = (GenericCanCan *)can;
  if (gcc->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanCan.");
  }
  CANMessage can_msg = { 0 };
  status_ok_or_return(generic_can_msg_to_can_message(msg, &can_msg));

  return can_transmit(&can_msg, NULL);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                  void *context) {
  GenericCanCan *gcc = (GenericCanCan *)can;
  if (gcc->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanCan.");
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_CAN_RX_HANDLERS; i++) {
    if (gcc->rx_storage[i].rx_handler == NULL && !gcc->rx_storage[i].enabled) {
      gcc->rx_storage[i].id = id;
      gcc->rx_storage[i].rx_handler = rx_handler;
      gcc->rx_storage[i].context = context;
      gcc->rx_storage[i].enabled = true;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}

static StatusCode prv_set_rx(GenericCan *can, uint32_t id, bool state) {
  GenericCanCan *gcc = (GenericCanCan *)can;
  if (gcc->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanCan.");
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_CAN_RX_HANDLERS; i++) {
    if (gcc->rx_storage[i].id == id && gcc->rx_storage[i].rx_handler != NULL) {
      gcc->rx_storage[i].enabled = state;
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

StatusCode generic_can_can_init(GenericCanCan *out) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;
  s_interface.enable_rx = prv_enable_rx;
  s_interface.disable_rx = prv_disable_rx;

  memset(out->rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_CAN_RX_HANDLERS);

  out->base.interface = &s_interface;

  status_ok_or_return(can_register_rx_default_handler(prv_generic_can_can_rx_handler, out));

  return STATUS_CODE_OK;
}
