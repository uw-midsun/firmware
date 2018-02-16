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

#define NO_DATA 0
#define CHAOS_CAN_NUM_RX_HANDLERS 1

static CANStorage s_can_storage;
static CANRxHandler s_can_rx_handlers[CHAOS_CAN_NUM_RX_HANDLERS];

// TODO(ELEC-105): Determine if there are any other messages we care about. BPS_FAULT comes to mind
// but if we are driving at 100 kph the driver not the car should switch to idle to cut power.

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
      event_raise(CHAOS_EVENT_SEQUENCE_IDLE, NO_DATA);
      break;
    case CHAOS_CAN_POWER_STATE_CHARGE:
      event_raise(CHAOS_EVENT_SEQUENCE_CHARGE, NO_DATA);
      break;
    case CHAOS_CAN_POWER_STATE_DRIVE:
      event_raise(CHAOS_EVENT_SEQUENCE_DRIVE, NO_DATA);
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
  return STATUS_CODE_OK;
}

bool chaos_can_process_event(const Event *e) {
  return fsm_process_event(CAN_FSM, e);
}
