#include "bps_heartbeat.h"
#include "can_transmit.h"
#include "log.h"
#include "plutus_cfg.h"

static StatusCode prv_handle_heartbeat_ack(CANMessageID msg_id, uint16_t device,
                                           CANAckStatus status, uint16_t num_remaining,
                                           void *context) {
  BpsHeartbeatStorage *storage = context;

  if (status != CAN_ACK_STATUS_OK) {
    // Something bad happened - fault
    storage->ack_fail_counter++;

    if (storage->ack_fail_counter >= PLUTUS_CFG_HEARTBEAT_MAX_ACK_FAILS) {
      return bps_heartbeat_raise_fault(storage, BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
    }
  } else if (status == CAN_ACK_STATUS_OK) {
    storage->ack_fail_counter = 0;
    return bps_heartbeat_clear_fault(storage, BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_handle_state(BpsHeartbeatStorage *storage) {
  CANAckRequest ack_request = {
    .callback = prv_handle_heartbeat_ack,
    .context = storage,
    .expected_bitset = storage->expected_bitset,
  };

  // Only transmit state OK if we have no ongoing faults
  EEBpsHeartbeatState state =
      (storage->fault_bitset == 0x0) ? EE_BPS_HEARTBEAT_STATE_OK : EE_BPS_HEARTBEAT_STATE_FAULT;
  if (state == EE_BPS_HEARTBEAT_STATE_FAULT) {
    LOG_DEBUG("fault: 0x%x\n", storage->fault_bitset);
  }
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

  // Assume things are okay until told otherwise?
  storage->fault_bitset = 0x00;
  storage->ack_fail_counter = 0;

  // TODO: need to make this a define or something?
  // this is required since chaos takes time to enter the main loop and process ACKs
  return soft_timer_start_millis(5000, prv_periodic_heartbeat, storage, NULL);
}

StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage, BpsHeartbeatFaultSource source) {
  storage->fault_bitset |= (1 << source);

  if (source == BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT) {
    return STATUS_CODE_OK;
  }

  // Only immediately send message if not due to ACK timeout
  return prv_handle_state(storage);
}

StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage, BpsHeartbeatFaultSource source) {
  storage->fault_bitset &= ~(1 << source);

  return STATUS_CODE_OK;
}
