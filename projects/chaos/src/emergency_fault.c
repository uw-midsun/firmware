#include "emergency_fault.h"

#include <stddef.h>
#include <stdint.h>

#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "critical_section.h"
#include "soft_timer.h"
#include "status.h"

// Forward declare to resolve the circular dependence of the functions.
static StatusCode prv_ack_handler(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                  uint16_t num_remaining, void *context);

// SoftTimerCallback
static void prv_send(SoftTimerID id, void *context) {
  (void)id;
  EmergencyFaultStorage *storage = context;
  storage->id = SOFT_TIMER_INVALID_TIMER;
  CANAckRequest req = {
    .callback = prv_ack_handler,
    .context = context,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_DRIVER_CONTROLS),
  };
  CAN_TRANSMIT_POWER_DISTRIBUTION_FAULT(&req);
}

// CANAckRequestCb
static StatusCode prv_ack_handler(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                  uint16_t num_remaining, void *context) {
  // Ignore this as there should only be one receiver.
  (void)msg_id;
  (void)device;
  EmergencyFaultStorage *storage = context;
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    // Ack was received so stop trying to send.
    storage->keep_trying = false;
  } else if (storage->keep_trying) {
    // Backoff to avoid spamming the CAN Bus.
    soft_timer_start_millis(EMERGENCY_FAULT_BACKOFF_MS, prv_send, context, &storage->id);
  }
  return STATUS_CODE_OK;
}

void emergency_fault_init(EmergencyFaultStorage *storage) {
  storage->id = SOFT_TIMER_INVALID_TIMER;
  storage->keep_trying = false;
}

StatusCode emergency_fault_send(EmergencyFaultStorage *storage) {
  storage->keep_trying = true;
  // Sends a fault message after a brief delay. This is just to simplify the behavior.
  return soft_timer_start_millis(1, prv_send, storage, &storage->id);
}

void emergency_fault_clear(EmergencyFaultStorage *storage) {
  bool disabled = critical_section_start();
  if (storage->id != SOFT_TIMER_INVALID_TIMER) {
    // Cancel the timer if it is running.
    soft_timer_cancel(storage->id);
    storage->id = SOFT_TIMER_INVALID_TIMER;
  }
  storage->keep_trying = false;
  critical_section_end(disabled);
}

void emergency_fault_process_event(EmergencyFaultStorage *storage, const Event *e) {
  switch (e->id) {
    case CHAOS_EVENT_SEQUENCE_EMERGENCY:
      emergency_fault_send(storage);
      break;
    case CHAOS_EVENT_SEQUENCE_IDLE:    // Falls through.
    case CHAOS_EVENT_SEQUENCE_CHARGE:  // Falls through.
    case CHAOS_EVENT_SEQUENCE_DRIVE:
      emergency_fault_clear(storage);
  }
}
