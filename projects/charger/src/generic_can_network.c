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
static StatusCode prv_generic_can_network_rx_handler(const CANMessage *msg, void *context,
                                                     CANAckStatus *ack_reply) {
  (void)ack_reply;
  GenericCanNetwork *gcn = context;
  GenericCanMsg generic_msg = { 0 };
  can_message_to_generic_can_message(msg, &generic_msg);
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (generic_msg.id == gcn->base.rx_storage[i].id &&
        gcn->base.rx_storage[i].rx_handler != NULL) {
      gcn->base.rx_storage[i].rx_handler(&generic_msg, gcn->base.rx_storage[i].context);
      break;
    }
  }
  return STATUS_CODE_OK;
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  }
  CANMessage can_msg = { 0 };
  status_ok_or_return(generic_can_msg_to_can_message(msg, &can_msg));

  return can_transmit(&can_msg, NULL);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                  void *context) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  }
  return generic_can_helpers_register_rx(can, rx_handler, id, context);
}

StatusCode generic_can_network_init(GenericCanNetwork *can_network) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;

  memset(can_network->base.rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_RX_HANDLERS);

  can_network->base.interface = &s_interface;
  status_ok_or_return(
      can_register_rx_default_handler(prv_generic_can_network_rx_handler, can_network));

  return STATUS_CODE_OK;
}
