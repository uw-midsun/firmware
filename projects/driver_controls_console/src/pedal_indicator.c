// To:do raise throttle related events
#include "pedal_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "cc_input_event.h"
#include "log.h"
#include "exported_enums.h"

static StatusCode prv_handle_pedal(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  DriveOutputStorage *storage = context;

  uint16_t throttle = 0;
  uint16_t mech_brake = 0;
  uint16_t throttle_state = 0;
  CAN_UNPACK_PEDAL_OUTPUT(msg, &throttle, &throttle_state, &mech_brake);

  storage->data[DRIVE_OUTPUT_SOURCE_THROTTLE] = (int16_t)throttle;
  storage->data[DRIVE_OUTPUT_SOURCE_MECH_BRAKE] = (int16_t)mech_brake;

  if (mech_brake > EE_PEDAL_OUTPUT_MECH_THRESHOLD) {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_MECH_BRAKE_PRESSED, mech_brake);
  } else {
    event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_MECH_BRAKE_RELEASED, mech_brake);
  }

  switch (throttle_state) {
    case EE_THROTTLE_BRAKE:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_BRAKE, 0);
      break;
    case EE_THROTTLE_COAST:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_COAST, 0);
      break;
    case EE_THROTTLE_ACCEL:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_ACCEL, 0);
      break;
    case EE_THROTTLE_FAULT:
      event_raise_priority(EVENT_PRIORITY_NORMAL, INPUT_EVENT_PEDAL_FAULT, 0);
      break;
    default:
      break;
  }

  return STATUS_CODE_OK;
}

StatusCode pedal_indicator_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_PEDAL_OUTPUT, prv_handle_pedal, NULL);
}
