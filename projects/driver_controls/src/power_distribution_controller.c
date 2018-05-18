#include "power_distribution_controller.h"

#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

// CANAckRequestCb
static StatusCode prv_ack_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                   uint16_t num_remaining, void *context) {
  // Only Chaos is expected to ack so this isn't important.
  (void)msg_id;
  (void)device;
  (void)num_remaining;
  (void)context;
  if (status != CAN_ACK_STATUS_OK) {
    // TODO(ELEC-105): Raise some kind of retry event.
  }
  return STATUS_CODE_OK;
}

StatusCode power_distribution_controller_send_update(EEPowerState power_state) {
  CANAckRequest req = {
    .callback = prv_ack_callback,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS),
  };
  switch (power_state) {
    case EE_POWER_STATE_IDLE:
      CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_IDLE);
      return STATUS_CODE_OK;
    case EE_POWER_STATE_CHARGE:
      CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_CHARGE);
      return STATUS_CODE_OK;
    case EE_POWER_STATE_DRIVE:
      CAN_TRANSMIT_POWER_STATE(&req, EE_POWER_STATE_DRIVE);
      return STATUS_CODE_OK;
    case NUM_EE_POWER_STATES:  // Falls through.
    default:
      return status_code(STATUS_CODE_INVALID_ARGS);
  }
}

bool power_distribution_controller_process_event(const Event *e);
