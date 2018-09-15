#include "bps_heartbeat.h"
#include "can_transmit.h"
#include "debug_led.h"
#include "log.h"
#include "plutus_cfg.h"

static StatusCode prv_handle_heartbeat_ack(CanMessageId msg_id, uint16_t device,
                                           CanAckStatus status, uint16_t num_remaining,
                                           void *context) {
  BpsHeartbeatStorage *storage = context;

  if (status != CAN_ACK_STATUS_OK) {
    // We missed an ACK - fault after some grace period
    storage->ack_fail_counter++;
    debug_led_set_state(DEBUG_LED_GREEN, false);

    if (storage->ack_fail_counter >= PLUTUS_CFG_HEARTBEAT_MAX_ACK_FAILS) {
      return bps_heartbeat_raise_fault(storage, EE_BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
    }
  } else if (num_remaining == 0) {
    // Received all ACKs as expected
    debug_led_set_state(DEBUG_LED_GREEN, true);

    storage->ack_fail_counter = 0;
    return bps_heartbeat_clear_fault(storage, EE_BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT);
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
  CAN_TRANSMIT_BPS_HEARTBEAT(&ack_request, storage->fault_bitset);
  debug_led_set_state(DEBUG_LED_RED, (storage->fault_bitset != EE_BPS_HEARTBEAT_STATE_OK));

  if (storage->fault_bitset != EE_BPS_HEARTBEAT_STATE_OK) {
    return sequenced_relay_set_state(storage->relay, EE_RELAY_STATE_OPEN);
  }

  return STATUS_CODE_OK;
}

static void prv_periodic_heartbeat(SoftTimerId timer_id, void *context) {
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

  // Turn the red LED on if we've faulted
  debug_led_init(DEBUG_LED_RED);
  // Turn the yellow LED on if we're receiving ACKs
  debug_led_init(DEBUG_LED_GREEN);

  return soft_timer_start_millis(storage->period_ms * BPS_HEARTBEAT_STARTUP_DELAY_MULTIPLIER,
                                 prv_periodic_heartbeat, storage, NULL);
}

StatusCode bps_heartbeat_raise_fault(BpsHeartbeatStorage *storage,
                                     EEBpsHeartbeatFaultSource source) {
  storage->fault_bitset |= (1 << source);

  if (source == EE_BPS_HEARTBEAT_FAULT_SOURCE_ACK_TIMEOUT) {
    return STATUS_CODE_OK;
  }

  // Only immediately send message if not due to ACK timeout
  return prv_handle_state(storage);
}

StatusCode bps_heartbeat_clear_fault(BpsHeartbeatStorage *storage,
                                     EEBpsHeartbeatFaultSource source) {
  storage->fault_bitset &= ~(1 << source);

  return STATUS_CODE_OK;
}
