#include "chaos_can.h"

#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "misc.h"
#include "status.h"

#define CHAOS_CAN_NO_DATA 0
#define CHAOS_CAN_NUM_RX_HANDLERS 2

static CANStorage s_can_storage;
static CANRxHandler s_can_rx_handlers[CHAOS_CAN_NUM_RX_HANDLERS];

static StatusCode prv_bps_fault_callback(const CANMessage *msg, void *context,
                                         CANAckStatus *ack_reply) {
  // This event will immediately transitions the car into the equivalent of the IDLE state only LV
  // systems powered off of the AUX Battery but it is irrecoverable without double IDLE requests.
  event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, CHAOS_CAN_NO_DATA);
  return STATUS_CODE_OK;
}

static StatusCode prv_power_state_callback(const CANMessage *msg, void *context,
                                           CANAckStatus *ack_reply) {
  ChaosCanPowerState power_state = NUM_CHAOS_CAN_POWER_STATES;
  CAN_UNPACK_POWER_STATE(msg, (uint8_t *)&power_state);
  switch (power_state) {
    // TODO(ELEC-105): If the selected state cannot be transitioned to consider returning invalid.
    // While not actionable this could at least provide feedback to the driver that the transition
    // selected was invalid. Theoretically this should be enforced on the driver controls side
    // though.
    case CHAOS_CAN_POWER_STATE_IDLE:
      event_raise(CHAOS_EVENT_SEQUENCE_IDLE, CHAOS_CAN_NO_DATA);
      break;
    case CHAOS_CAN_POWER_STATE_CHARGE:
      event_raise(CHAOS_EVENT_SEQUENCE_CHARGE, CHAOS_CAN_NO_DATA);
      break;
    case CHAOS_CAN_POWER_STATE_DRIVE:
      event_raise(CHAOS_EVENT_SEQUENCE_DRIVE, CHAOS_CAN_NO_DATA);
      break;
    case NUM_CHAOS_CAN_POWER_STATES:
    default:
      *ack_reply = CAN_ACK_STATUS_INVALID;
      return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

StatusCode chaos_can_init(CANSettings *settings) {
  status_ok_or_return(
      can_init(settings, &s_can_storage, s_can_rx_handlers, SIZEOF_ARRAY(s_can_rx_handlers)));

  status_ok_or_return(
      can_register_rx_handler(CAN_MESSAGE_POWER_STATE, prv_power_state_callback, NULL));
  status_ok_or_return(can_register_rx_handler(CAN_MESSAGE_BPS_FAULT, prv_bps_fault_callback, NULL));
  return STATUS_CODE_OK;
}

bool chaos_can_process_event(const Event *e) {
  return fsm_process_event(CAN_FSM, e);
}
