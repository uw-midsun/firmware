//To:do raise control stalk related events ( analog and digital mapping)
#include "steering_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "cc_input_event.h"

static StatusCode prv_handle_steering(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  DriveOutputStorage *storage = context;

  uint16_t cruise = 0;

  CAN_UNPACK_STEERING_OUTPUT(msg, &cruise);

  storage->data[DRIVE_OUTPUT_SOURCE_CRUISE] = (int16_t)cruise;

  return STATUS_CODE_OK;
}

StatusCode steering_indicator_init(void) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_OUTPUT, prv_handle_steering,
                                 drive_output_global());
}
