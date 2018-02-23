#include "relay_fsm.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "misc.h"
#include "relay_id.h"
#include "status.h"

typedef struct RelayFsmAckCtx {
  RelayId id;
  EventID event_id;
  uint8_t retries;
} RelayFsmAckCtx;

typedef struct RelayFsmCtx {
  RelayFsmAckCtx ack_ctx;
  CANAckRequest request;
} RelayFsmCtx;

static RelayFsmCtx s_fsm_ctxs[NUM_RELAY_FSMS];

// Select whether the FSM is configured to handle that specific relay. The Event data field should
// be the RelayId of the FSM that needs to be transitioned.
static bool prv_guard_select_relay(const FSM *fsm, const Event *e, void *context) {
  RelayFsmCtx *fsm_ctx = context;
  return e->data == fsm_ctx->ack_ctx.id;
}

static StatusCode prv_ack_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                   uint16_t num_remaining, void *context) {
  RelayFsmAckCtx *ack_ctx = context;
  if (status != CAN_ACK_STATUS_OK) {
    if (ack_ctx->retries < RELAY_FSM_MAX_RETRIES) {
      ack_ctx->retries++;
      event_raise(CHAOS_EVENT_RETRY_RELAY, ack_ctx->id);
    } else {
      ack_ctx->retries = 0;
      event_raise(CHAOS_EVENT_RELAY_ERROR, ack_ctx->id);
    }
  } else {
    ack_ctx->retries = 0;
    event_raise(ack_ctx->event_id, ack_ctx->id);
  }

  return STATUS_CODE_OK;
}

FSM_DECLARE_STATE(relay_opened);

FSM_DECLARE_STATE(relay_closing);

FSM_DECLARE_STATE(relay_closed);

FSM_DECLARE_STATE(relay_opening);

FSM_STATE_TRANSITION(relay_opened) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_CLOSE_RELAY, prv_guard_select_relay, relay_closing);
}

FSM_STATE_TRANSITION(relay_closing) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_CLOSED, prv_guard_select_relay, relay_closed);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard_select_relay, relay_closing);
}

FSM_STATE_TRANSITION(relay_closed) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_OPEN_RELAY, prv_guard_select_relay, relay_opening);
}

FSM_STATE_TRANSITION(relay_opening) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_OPENED, prv_guard_select_relay, relay_opened);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard_select_relay, relay_opening);
}

static void prv_relay_transmit(RelayId id, RelayState state, const CANAckRequest *ack_request) {
  switch (id) {
    case RELAY_ID_SOLAR_MASTER_FRONT:
      // TODO(ELEC-105): Replace with solar front/back.
      CAN_TRANSMIT_SOLAR_RELAY(ack_request, state);
      return;
    case RELAY_ID_SOLAR_MASTER_REAR:
      CAN_TRANSMIT_SOLAR_RELAY(ack_request, state);
      return;
    case RELAY_ID_BATTERY:
      CAN_TRANSMIT_BATTERY_RELAY(ack_request, state);
      return;
    case RELAY_ID_MAIN_POWER:
      CAN_TRANSMIT_MAIN_RELAY(ack_request, state);
      return;
    case NUM_RELAY_IDS:
    default:
      return;
  }
}

static void prv_opening(FSM *fsm, const Event *e, void *context) {
  RelayFsmCtx *relay_ctx = context;
  relay_ctx->ack_ctx.event_id = CHAOS_EVENT_RELAY_OPENED;
  prv_relay_transmit(relay_ctx->ack_ctx.id, RELAY_STATE_OPEN, &relay_ctx->request);
}

static void prv_closing(FSM *fsm, const Event *e, void *context) {
  RelayFsmCtx *relay_ctx = context;
  relay_ctx->ack_ctx.event_id = CHAOS_EVENT_RELAY_CLOSED;
  prv_relay_transmit(relay_ctx->ack_ctx.id, RELAY_STATE_CLOSE, &relay_ctx->request);
}

void relay_fsm_init(void) {
  fsm_state_init(relay_opening, prv_opening);
  fsm_state_init(relay_closing, prv_closing);

  memset(s_fsm_ctxs, 0, sizeof(s_fsm_ctxs));
}

StatusCode relay_fsm_create(FSM *fsm, RelayId relay_id, const char *fsm_name,
                            uint32_t ack_device_bitset) {
  if (relay_id > NUM_RELAY_IDS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  s_fsm_ctxs[relay_id].ack_ctx.id = relay_id;
  s_fsm_ctxs[relay_id].ack_ctx.retries = 0;
  s_fsm_ctxs[relay_id].request.callback = prv_ack_callback;
  s_fsm_ctxs[relay_id].request.expected_bitset = ack_device_bitset;
  s_fsm_ctxs[relay_id].request.context = &s_fsm_ctxs[relay_id].ack_ctx;
  fsm_init(fsm, fsm_name, &relay_opened, &s_fsm_ctxs[relay_id]);
  return STATUS_CODE_OK;
}

StatusCode relay_fsm_open_event(RelayId relay_id, Event *e) {
  if (relay_id >= NUM_RELAY_IDS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  e->id = CHAOS_EVENT_OPEN_RELAY;
  e->data = relay_id;
  return STATUS_CODE_OK;
}

StatusCode relay_fsm_close_event(RelayId relay_id, Event *e) {
  if (relay_id >= NUM_RELAY_IDS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  e->id = CHAOS_EVENT_CLOSE_RELAY;
  e->data = relay_id;
  return STATUS_CODE_OK;
}
