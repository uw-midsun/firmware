#include "power_distribution_controller.h"

#include <stddef.h>
#include <stdint.h>

#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "cc_input_event.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "status.h"

static EEPowerState s_last_state = NUM_EE_POWER_STATES;

// CanAckRequestCb
static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  // Only Chaos is expected to ack so this isn't important.
  (void)msg_id;
  (void)device;
  (void)num_remaining;
  (void)context;
  if (status == CAN_ACK_STATUS_TIMEOUT) {
    event_raise(INPUT_EVENT_CENTER_CONSOLE_BPS_FAULT, 0);
  } else if (status == CAN_ACK_STATUS_INVALID) {
    event_raise(INPUT_EVENT_CENTER_CONSOLE_RETRY_POWER_STATE, s_last_state);
  }
  return STATUS_CODE_OK;
}

StatusCode power_distribution_controller_send_update(EEPowerState power_state) {
  if (power_state >= NUM_EE_POWER_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS),
  };
  s_last_state = power_state;
  return CAN_TRANSMIT_POWER_STATE(&req, power_state);
}

StatusCode power_distribution_controller_retry(const Event *e) {
  if (e->id == INPUT_EVENT_CENTER_CONSOLE_RETRY_POWER_STATE) {
    if (e->data <= NUM_EE_POWER_STATES) {
      return power_distribution_controller_send_update(e->data);
    }
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}
