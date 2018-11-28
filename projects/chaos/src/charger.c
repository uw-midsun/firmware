#include "charger.h"

#include <stdlib.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "status.h"

typedef struct ChargerStorage {
  EEChargerSetRelayState relay_state;
  EEChargerConnState conn_state;
} ChargerStorage;

static ChargerStorage s_storage;

// CanRxHandlerCb
static StatusCode prv_handle_charger_conn_state(const CanMessage *msg, void *context,
                                                CanAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  CAN_UNPACK_CHARGER_CONN_STATE(msg, (uint8_t *)&s_storage.conn_state);
  charger_set_state(s_storage.relay_state);
  return STATUS_CODE_OK;
}

StatusCode charger_init(void) {
  s_storage.relay_state = EE_CHARGER_SET_RELAY_STATE_OPEN;
  s_storage.conn_state = EE_CHARGER_CONN_STATE_DISCONNECTED;

  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGER_CONN_STATE,
                                 prv_handle_charger_conn_state, NULL);
}

StatusCode charger_set_state(EEChargerSetRelayState state) {
  if (state >= NUM_EE_CHARGER_SET_RELAY_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_storage.relay_state = state;
  if (s_storage.conn_state == EE_CHARGER_CONN_STATE_CONNECTED) {
    return CAN_TRANSMIT_CHARGER_SET_RELAY_STATE((uint8_t)state);
  }
  return STATUS_CODE_OK;
}

bool charger_process_event(const Event *e) {
  if (e->id != CHAOS_EVENT_CHARGER_OPEN && e->id != CHAOS_EVENT_CHARGER_CLOSE) {
    return false;
  }

  if (e->id == CHAOS_EVENT_CHARGER_OPEN) {
    charger_set_state(EE_CHARGER_SET_RELAY_STATE_OPEN);
  } else {
    charger_set_state(EE_CHARGER_SET_RELAY_STATE_CLOSE);
  }
  return true;
}
