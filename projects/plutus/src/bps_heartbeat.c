#include "bps_heartbeat.h"
#include "can_transmit.h"

static StatusCode prv_handle_heartbeat_ack(CANMessageID msg_id, uint16_t device,
                                           CANAckStatus status, uint16_t num_remaining,
                                           void *context) {
  BpsHeartbeatStorage *storage = context;

  if (status != CAN_ACK_STATUS_OK) {
    // Something bad happened - fault
    return bps_heartbeat_raise_fault(storage);
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_handle_state(BpsHeartbeatStorage *storage) {
  CANAckRequest ack_request = {
    .callback = prv_handle_heartbeat_ack,
    .context = storage,
    .expected_bitset = storage->expected_bitset,
  };

  EEBpsHeartbeatState state =
      (storage->faulted) ? EE_BPS_HEARTBEAT_STATE_FAULT : EE_BPS_HEARTBEAT_STATE_OK;
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_request, state);

  if (state == EE_BPS_HEARTBEAT_STATE_FAULT) {
    return sequenced_relay_set_state(storage->relay, EE_RELAY_STATE_OPEN);
  }

  return STATUS_CODE_OK;
}

static void prv_periodic_heartbeat(SoftTimerID timer_id, void *context) {
  BpsHeartbeatStorage *storage = context;

  prv_handle_state(storage);

  soft_timer_start_millis(storage->period_ms, prv_periodic_heartbeat, storage, NULL);
}

StatusCode bps_heartbeat_init(BpsHeartbeatStorage *storage, SequencedRelayStorage *relay,
                              uint32_t period_ms, uint32_t expected_bitset) {
  storage->relay = relay;
  storage->period_ms = period_ms;
  storage->expected_bitset = expected_bitset;
  storage->faulted = false;

  return soft_timer_start_millis(storage->period_ms, prv_periodic_heartbeat, storage, NULL);
}

StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage) {
  storage->faulted = true;

  return prv_handle_state(storage);
}

StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage) {
  storage->faulted = false;

  return STATUS_CODE_OK;
}
