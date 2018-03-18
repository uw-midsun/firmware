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
  GenericCanRxStorage *rxs = context;
  if (!rxs->enabled) {
    return STATUS_CODE_OK;
  }
  GenericCanMsg generic_msg = { 0 };
  can_message_to_generic_can_message(msg, &generic_msg);

  rxs->rx_handler(&generic_msg, rxs->context);

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

  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (gcn->base.rx_storage[i].rx_handler == NULL && !gcn->base.rx_storage[i].enabled) {
      gcn->base.rx_storage[i].id = id;
      gcn->base.rx_storage[i].rx_handler = rx_handler;
      gcn->base.rx_storage[i].context = context;
      gcn->base.rx_storage[i].enabled = true;
      status_ok_or_return(can_register_rx_handler(id, prv_generic_can_network_rx_handler,
                                                  &gcn->base.rx_storage[i]));
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}

// enable_rx
static StatusCode prv_enable_rx(GenericCan *can, uint32_t id) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  }
  return generic_can_helpers_set_rx(can, id, true);
}

// disable_rx
static StatusCode prv_disable_rx(GenericCan *can, uint32_t id) {
  GenericCanNetwork *gcn = (GenericCanNetwork *)can;
  if (gcn->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanNetwork.");
  }
  return generic_can_helpers_set_rx(can, id, false);
}

StatusCode generic_can_network_init(GenericCanNetwork *can_network) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;
  s_interface.enable_rx = prv_enable_rx;
  s_interface.disable_rx = prv_disable_rx;

  memset(can_network->base.rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_RX_HANDLERS);

  can_network->base.interface = &s_interface;

  return STATUS_CODE_OK;
}
