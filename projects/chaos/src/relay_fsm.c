#include "relay_fsm.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "can.h"
#include "log.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "critical_section.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "misc.h"
#include "relay_id.h"
#include "status.h"

typedef struct RelayFsmAckCtx {
  RelayId id;
  EventId event_id;
} RelayFsmAckCtx;

typedef struct RelayFsmCtx {
  RelayFsmAckCtx ack_ctx;
  CanAckRequest request;
  GpioAddress power_pin;
} RelayFsmCtx;

static char *relay_name_lookup[NUM_RELAY_IDS] = {
  [RELAY_ID_SOLAR_MASTER_FRONT] = "Solar Master Front",
  [RELAY_ID_SOLAR_MASTER_REAR] = "Solar Master Rear",
  [RELAY_ID_BATTERY_MAIN] = "Battery Main",
  [RELAY_ID_BATTERY_SLAVE] = "Battery Slave",
  [RELAY_ID_MOTORS] = "Motors"
};

static RelayFsmCtx s_fsm_ctxs[NUM_RELAY_FSMS];

// Select whether the FSM is configured to handle that specific relay. The Event data field should
// be the RelayId of the FSM that needs to be transitioned.
static bool prv_guard_select_relay(const Fsm *fsm, const Event *e, void *context) {
  RelayFsmCtx *fsm_ctx = context;
  return e->data == fsm_ctx->ack_ctx.id;
}

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)msg_id;
  (void)device;
  (void)num_remaining;
  RelayFsmAckCtx *ack_ctx = context;
  if (status != CAN_ACK_STATUS_OK) {
    event_raise(CHAOS_EVENT_MAYBE_RETRY_RELAY, ack_ctx->id);
  } else {
    event_raise(ack_ctx->event_id, ack_ctx->id);
  }
  return STATUS_CODE_OK;
}

FSM_DECLARE_STATE(relay_opened);

FSM_DECLARE_STATE(relay_closing);

FSM_DECLARE_STATE(relay_closed);

FSM_DECLARE_STATE(relay_opening);

FSM_STATE_TRANSITION(relay_opened) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_OPEN_RELAY, prv_guard_select_relay, relay_opening);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_CLOSE_RELAY, prv_guard_select_relay, relay_closing);
}

FSM_STATE_TRANSITION(relay_closing) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_CLOSED, prv_guard_select_relay, relay_closed);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard_select_relay, relay_closing);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_ERROR, prv_guard_select_relay, relay_opened);
}

FSM_STATE_TRANSITION(relay_closed) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_OPEN_RELAY, prv_guard_select_relay, relay_opening);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_CLOSE_RELAY, prv_guard_select_relay, relay_closing);
}

FSM_STATE_TRANSITION(relay_opening) {
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_OPENED, prv_guard_select_relay, relay_opened);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard_select_relay, relay_opening);
  FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_ERROR, prv_guard_select_relay, relay_closed);
}

static void prv_relay_transmit(RelayId id, RelayState state, const CanAckRequest *ack_request) {
  switch (id) {
    case RELAY_ID_SOLAR_MASTER_FRONT:
      CAN_TRANSMIT_SOLAR_RELAY_FRONT(ack_request, state);
      return;
    case RELAY_ID_SOLAR_MASTER_REAR:
      CAN_TRANSMIT_SOLAR_RELAY_REAR(ack_request, state);
      return;
    case RELAY_ID_BATTERY_MAIN:
      CAN_TRANSMIT_BATTERY_RELAY_MAIN(ack_request, state);
      return;
    case RELAY_ID_BATTERY_SLAVE:
      CAN_TRANSMIT_BATTERY_RELAY_SLAVE(ack_request, state);
      return;
    case RELAY_ID_MOTORS:
      CAN_TRANSMIT_MOTOR_RELAY(ack_request, state);
      return;
    case NUM_RELAY_IDS:  // Falls through.
    default:
      return;
  }
}

static void prv_opening(Fsm *fsm, const Event *e, void *context) {
  RelayFsmCtx *relay_ctx = context;
  // Check that the GPIO pin is in |GPIO_STATE_HIGH| before opening. If it is already off we assume
  // the relay to be opened already.
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&relay_ctx->power_pin, &state);
  if (state == GPIO_STATE_LOW) {
    event_raise(CHAOS_EVENT_RELAY_OPENED, relay_ctx->ack_ctx.id);
    RelayId id = (relay_ctx->ack_ctx).id;
    LOG_DEBUG("Relay %s opened.\n", relay_name_lookup[id]);
    return;
  }
  relay_ctx->ack_ctx.event_id = CHAOS_EVENT_RELAY_OPENED;
  prv_relay_transmit(relay_ctx->ack_ctx.id, (RelayState)EE_RELAY_STATE_OPEN, &relay_ctx->request);
}

static void prv_closing(Fsm *fsm, const Event *e, void *context) {
  RelayFsmCtx *relay_ctx = context;
  // Check that the GPIO pin is in |GPIO_STATE_HIGH| before closing. If it is already off this
  // action is an error and we raise it as such, this will reset us to the open state.
  GpioState state = GPIO_STATE_LOW;
  gpio_get_state(&relay_ctx->power_pin, &state);
  if (state == GPIO_STATE_LOW) {
    event_raise(CHAOS_EVENT_RELAY_ERROR, relay_ctx->ack_ctx.id);
    return;
  }
  relay_ctx->ack_ctx.event_id = CHAOS_EVENT_RELAY_CLOSED;
  RelayId id = (relay_ctx->ack_ctx).id;
  LOG_DEBUG("Relay %s closed.\n", relay_name_lookup[id]);
  prv_relay_transmit(relay_ctx->ack_ctx.id, (RelayState)EE_RELAY_STATE_CLOSE, &relay_ctx->request);
}

void relay_fsm_init(void) {
  fsm_state_init(relay_opening, prv_opening);
  fsm_state_init(relay_closing, prv_closing);

  memset(s_fsm_ctxs, 0, sizeof(s_fsm_ctxs));
}

StatusCode relay_fsm_create(Fsm *fsm, RelayId relay_id, const char *fsm_name,
                            const GpioAddress *addr, uint32_t ack_device_bitset) {
  if (relay_id > NUM_RELAY_IDS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  s_fsm_ctxs[relay_id].ack_ctx.id = relay_id;
  s_fsm_ctxs[relay_id].request.callback = prv_ack_callback;
  s_fsm_ctxs[relay_id].request.expected_bitset = ack_device_bitset;
  s_fsm_ctxs[relay_id].request.context = &s_fsm_ctxs[relay_id].ack_ctx;
  s_fsm_ctxs[relay_id].power_pin.port = addr->port;
  s_fsm_ctxs[relay_id].power_pin.pin = addr->pin;
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
