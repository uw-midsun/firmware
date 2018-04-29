#include "charger.h"

#include <stdlib.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "status.h"

typedef enum {
  CHARGER_STATUS_DISCONNECTED = 0,
  CHARGER_STATUS_CONNECTED,
  NUM_CHARGER_STATUSES,
} ChargerStatus;

typedef struct ChargerStorage {
  ChargerState state;
  ChargerStatus status;
} ChargerStorage;

static ChargerStorage s_storage;

// CanRxHandlerCb
static StatusCode prv_handle_charger_status(const CANMessage *msg, void *context,
                                            CANAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  CAN_UNPACK_CHARGING_REQ(msg);
  charger_set_state(s_storage.state);
}

StatusCode charger_init(void) {
  s_storage.state = CHARGER_STATE_DISABLED;
  s_storage.status = CHARGER_STATUS_DISCONNECTED;

  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGING_REQ, prv_handle_charger_status, NULL);
}

StatusCode charger_set_state(ChargerState state) {
  if (state >= NUM_CHARGER_STATUSES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_storage.state = state;
  if (s_storage.status == CHARGER_STATUS_CONNECTED) {
    return CAN_TRANSMIT_CHARGING_PERMISSION(state);
  }
  return STATUS_CODE_OK;
}
