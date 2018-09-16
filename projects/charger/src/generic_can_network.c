#include "generic_can_network.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can.h"
#include "generic_can.h"
#include "generic_can_helpers.h"
#include "generic_can_msg.h"
#include "log.h"
#include "status.h"

static GenericCanInterface s_interface;

// CanRxHandlerCb
static StatusCode prv_generic_can_network_rx_handler(const CanMessage *msg, void *context,
                                                     CanAckStatus *ack_reply) {
  (void)ack_reply;
  GenericCanRxStorage *gcrx = context;
  GenericCanMsg generic_msg = { 0 };
  can_message_to_generic_can_message(msg, &generic_msg);
  gcrx->rx_handler(&generic_msg, gcrx->context);
  return STATUS_CODE_OK;
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  }
  CanMessage can_msg = { 0 };
  status_ok_or_return(generic_can_msg_to_can_message(msg, &can_msg));

  return can_transmit(&can_msg, NULL);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t mask,
                                  uint32_t filter, bool extended, void *context) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  } else if (extended) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t idx = UINT16_MAX;
  status_ok_or_return(
      generic_can_helpers_register_rx(can, rx_handler, mask, filter, context, &idx));
  return can_register_rx_handler(filter, prv_generic_can_network_rx_handler,
                                 &gcn->base.rx_storage[idx]);
}

StatusCode generic_can_network_init(GenericCanNetwork *can_network) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;

  memset(can_network->base.rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_RX_HANDLERS);

  can_network->base.interface = &s_interface;

  return STATUS_CODE_OK;
}
