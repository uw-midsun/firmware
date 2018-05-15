#include "relay_rx.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "critical_section.h"
#include "exported_enums.h"
#include "gpio.h"
#include "status.h"

static RelayRxStorage *s_storage;
static size_t s_storage_size;
static size_t s_storage_idx;

// CANRxHandler
static StatusCode prv_relay_rx_can_handler(const CANMessage *msg, void *context,
                                           CANAckStatus *ack_reply) {
  RelayRxStorage *storage = context;
  EEChaosCmdRelayState state = NUM_EE_CHAOS_CMD_RELAY_STATES;
  // NOTE: This is a bit of a hack that exploits the fact all the relay control messages are the
  // same. The aim here is to not necessarily force a constraint based on message id/name. Instead
  // all single field u8 messages will succeed in unpacking but the contents must be valid.
  CAN_UNPACK_BATTERY_RELAY(msg, (uint8_t *)&state);
  if (state >= NUM_EE_CHAOS_CMD_RELAY_STATES) {
    *ack_reply = CAN_ACK_STATUS_INVALID;
  } else {
    storage->curr_state = state;
    if (!status_ok(storage->handler(storage->msg_id, storage->curr_state, storage->context))) {
      *ack_reply = CAN_ACK_STATUS_INVALID;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode relay_rx_init(RelayRxStorage *relay_storage, size_t size) {
  if (size == 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_storage = relay_storage;
  memset(relay_storage, 0, size);
  s_storage_size = size;
  s_storage_idx = 0;
  return STATUS_CODE_OK;
}

StatusCode relay_rx_configure_handler(SystemCanMessage msg_id, RelayRxHandler handler,
                                      void *context) {
  if (s_storage_idx >= s_storage_size) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (handler == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // NOTE: we explicitly don't constrain |msg_id|. In theory we could force this to be a value
  // defined for one of the relays in codegen-tooling. But in theory this module could be used for
  // any CAN controlled GPIO. Also note that the storage is also extensible for this reason.

  bool disabled_in_scope = critical_section_start();
  StatusCode status =
      can_register_rx_handler(msg_id, prv_relay_rx_can_handler, &s_storage[s_storage_idx]);
  if (!status_ok(status)) {
    critical_section_end(disabled_in_scope);
    return status;
  }

  s_storage[s_storage_idx].handler = handler;
  s_storage[s_storage_idx].msg_id = msg_id;
  s_storage[s_storage_idx].curr_state = EE_CHAOS_CMD_RELAY_STATE_OPEN;
  s_storage[s_storage_idx].context = context;
  s_storage_idx++;
  critical_section_end(disabled_in_scope);
  return STATUS_CODE_OK;
}
